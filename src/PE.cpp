#include <PE.hpp>

#include <cstdio>
#include <cstring>
#include <unordered_set>

std::unordered_map<std::string, SectionType> section_names = {
    {".debug", SectionType::Debug},
    {".drectve", SectionType::Drectve},
    {".edata", SectionType::Edata},
    {".idata", SectionType::Idata},
    {".pdata", SectionType::Pdata},
    {".reloc", SectionType::Reloc},
    {".tls", SectionType::TLS},
    {".rsrc", SectionType::Rsrc},
    {".cormeta", SectionType::Cormeta},
    {".sxdata", SectionType::SXData},
};

bool PE::load(char const* data, bool writable)
{
    uint32_t offset;
    memcpy(&offset, data + 0x3c, 4);

    uint32_t sig;
    memcpy(&sig, data + offset, 4);

    // "PE\0\0"
    valid_ = sig == 0x4550;

    if (!valid_)
    {
        return false;
    }

    offset += 4;
    header_ = (COFFHeader const*)(data + offset);
    if ((uint32_t)header_->characteristics
        & (uint32_t)Characteristics::ExecutableImage == 0)
    {
        std::fprintf(stderr, "PE loaded is not a valid executable file.");
        return false;
    }
    offset += sizeof(COFFHeader);

    uint32_t optional_header_offset = offset;

    optional_header_ = (OptionalHeader const*)(data + offset);
    offset += sizeof(OptionalHeader);

    uint32_t data_base;
    if (optional_header_->magic == OptionalHeaderMagic::PE32)
    {
        memcpy(&data_base, data + offset, 4);
        offset += 4;

        win32_header_ = (OptionalWindowsHeader32 const*)(data + offset);
        offset += sizeof(OptionalWindowsHeader32);
    }
    else
    {
        win32_plus_header_ = (OptionalWindowsHeader32Plus const*)(data + offset);
        offset += sizeof(OptionalWindowsHeader32Plus);
    }

    uint32_t sections = directory_count();

    for (uint32_t i = 0; i != sections; ++i)
    {
        directories[i] = (ImageDataDirectory const*)(data + offset);
        offset += sizeof(ImageDataDirectory);
    }
    for (uint32_t i = sections; i != (uint32_t)DataDirectoryType::COUNT; ++i)
    {
        directories[i] = nullptr;
    }

    if (offset - optional_header_offset != header_->optional_header_size)
    {
        std::fprintf(stderr, "PE header parsing inconsistency detected.");
        return false;
    }

    section_headers_.resize(header_->section_count);
    for (uint16_t i = 0; i != header_->section_count; ++i)
    {
        section_headers_[i] = (SectionHeader const*)(data + offset);
        offset += sizeof(SectionHeader);

        auto it = section_names.find(std::string{section_headers_[i]->name});
        if (it != section_names.end())
        {
            section_index_[it->second] = section_headers_[i];
        }
    }

    data_ = data;

    return true;
}

uint32_t PE::directory_count() const
{
    if (win32_header_)
    {
        return win32_header_->rva_and_size_count;
    }
    else if (win32_plus_header_)
    {
        return win32_plus_header_->rva_and_size_count;
    }
    return 0;
}

void PE::examine_imports()
{
    std::printf("Imports:\n\n");

    // https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#the-reloc-section-image-only
    auto it = section_index_.find(SectionType::Reloc);
    if (it == section_index_.end())
    {
        return;
    }

    ImageDataDirectory const*
        import_dir = directories[(int)DataDirectoryType::Import];

    uint32_t import_data_directory_offset = resolve_rva(import_dir->rva);

    ImportDirectoryEntry const*
        entries = (ImportDirectoryEntry const*)(data_
                                                + import_data_directory_offset);

    int index = 0;
    ImportDirectoryEntry null_entry{};
    while (true)
    {
        ImportDirectoryEntry const* entry = entries + index;

        if (memcmp(entry, &null_entry, sizeof(ImportDirectoryEntry)) == 0)
        {
            break;
        }

        if (entry->name_rva != 0)
        {
            // These names will resolve in either .reloc or .idata typically
            char const* name = (char const*)(data_
                                             + resolve_rva(entry->name_rva));
            std::printf("    %s\n", name);
        }

        ++index;
    }
}

bool PE::escalate(std::vector<std::string> const& dlls, char* out_data)
{
    bool error = false;
    std::unordered_set<std::string> uniq_dlls;
    for (std::string const& dll : dlls)
    {
        if (uniq_dlls.contains(dll))
        {
            std::fprintf(stderr,
                         "Escalation list contains duplicate entry %s\n",
                         dll.c_str());
            error = true;
        }
        else
        {
            uniq_dlls.insert(dll);
        }
    }

    if (error)
    {
        return false;
    }

    uint32_t file_offset;
    std::vector<ImportDirectoryEntry> entries = extract_imports(file_offset);

    // Ensure all DLLs requested for escalation are present in the import
    // directory.

    std::unordered_map<std::string, ImportDirectoryEntry const*> entries_by_name;

    std::printf("Original import list:\n");

    for (ImportDirectoryEntry const& entry : entries)
    {
        std::string name = (char const*)(data_ + resolve_rva(entry.name_rva));
        if (uniq_dlls.contains(name))
        {
            uniq_dlls.erase(name);
        }

        std::printf("    %s\n", name.c_str());

        entries_by_name[name] = &entry;
    }

    std::printf("\n");

    if (!uniq_dlls.empty())
    {
        std::fprintf(stderr,
                     "One or more DLLs requested for escalation were not "
                     "present in the PE import directory:\n");

        for (std::string const& dll : uniq_dlls)
        {
            std::fprintf(stderr, "    %s\n", dll.c_str());
        }

        return false;
    }

    // Reorder the import entry list, prioritizing the escalated DLLs

    std::vector<ImportDirectoryEntry> reordered;
    reordered.reserve(entries.size());

    for (std::string const& dll : dlls)
    {
        auto it = entries_by_name.find(dll);
        reordered.push_back(*it->second);
        entries_by_name.erase(it);
    }

    for (ImportDirectoryEntry const& entry : entries)
    {
        std::string name = (char const*)(data_ + resolve_rva(entry.name_rva));
        auto it          = entries_by_name.find(name);
        if (it != entries_by_name.end())
        {
            reordered.push_back(*it->second);
        }
    }

    std::printf("Reordered import list:\n");
    for (ImportDirectoryEntry const& entry : reordered)
    {
        std::string name = (char const*)(data_ + resolve_rva(entry.name_rva));
        std::printf("    %s\n", name.c_str());
    }

    if (!out_data)
    {
        // Dry-run assumed
        return true;
    }
    else
    {
        memcpy(out_data + file_offset,
               (void const*)reordered.data(),
               reordered.size() * sizeof(ImportDirectoryEntry));
    }

    return true;
}

std::vector<ImportDirectoryEntry> PE::extract_imports(uint32_t& file_offset)
{
    std::vector<ImportDirectoryEntry> result;

    ImageDataDirectory const*
        import_dir = directories[(int)DataDirectoryType::Import];

    file_offset = resolve_rva(import_dir->rva);

    ImportDirectoryEntry const*
        entries = (ImportDirectoryEntry const*)(data_ + file_offset);

    int index = 0;
    ImportDirectoryEntry null_entry{};
    while (true)
    {
        ImportDirectoryEntry const* entry = entries + index;

        if (memcmp(entry, &null_entry, sizeof(ImportDirectoryEntry)) == 0)
        {
            break;
        }

        result.push_back(*entry);
        ++index;
    }

    return result;
}

uint32_t PE::resolve_rva(uint32_t rva)
{
    for (SectionHeader const* section : section_headers_)
    {
        if (rva >= section->virtual_address
            && rva < section->virtual_address + section->virtual_size)
        {
            return rva - section->virtual_address + section->raw_data_offset;
        }
    }
    return 0;
}
