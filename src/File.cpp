#include <File.hpp>

#include <Windows.h>
#include <cstdio>

File::~File()
{
    reset();
}

bool File::load(std::string path, bool writable)
{
    reset();

    path_ = path;

    // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
    if (writable)
    {
        file_ = CreateFileA(path_.c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            nullptr,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            nullptr);
    }
    else
    {
        file_ = CreateFileA(path_.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            nullptr,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_READONLY,
                            nullptr);
    }

    if (file_ == INVALID_HANDLE_VALUE)
    {
        std::fprintf(stderr, "Failed to open file %s\n", path.c_str());
        file_ = nullptr;
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfilemappinga
    if (writable)
    {
        mapping_ = CreateFileMappingA(
            file_, nullptr, PAGE_READWRITE, 0, 0, nullptr);
    }
    else
    {
        mapping_ = CreateFileMappingA(
            file_, nullptr, PAGE_READONLY, 0, 0, nullptr);
    }

    if (!mapping_)
    {
        std::fprintf(
            stderr, "Failed to create mapping for file %s\n", path.c_str());
        return false;
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
    if (writable)
    {
        mutable_data_ = (char*)MapViewOfFile(
            mapping_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        data_ = mutable_data_;
    }
    else
    {
        data_ = (char const*)MapViewOfFile(mapping_, FILE_MAP_READ, 0, 0, 0);
    }

    if (!data_)
    {
        std::fprintf(stderr, "Failed to map view for file %s\n", path.c_str());
        return false;
    }

    return true;
}

void File::reset()
{
    if (data_)
    {
        UnmapViewOfFile(data_);
        data_ = nullptr;
    }

    if (mapping_)
    {
        CloseHandle(mapping_);
        mapping_ = nullptr;
    }

    if (file_)
    {
        CloseHandle(file_);
        file_ = nullptr;
    }
}
