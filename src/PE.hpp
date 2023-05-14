#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#machine-types
enum class MachineType : uint16_t
{
    Unknown     = 0x0,
    Alpha       = 0x184,
    Alpha64     = 0x284,
    AM33        = 0x1d3,
    AMD64       = 0x8664,
    ARM         = 0x1c0,
    ARM64       = 0xaa64,
    ARMNT       = 0x1c4,
    AXP64       = 0x284,
    EBC         = 0xebc,
    I386        = 0x14c,
    IA64        = 0x200,
    LoongArch32 = 0x6232,
    LoongArch64 = 0x6264,
    M32R        = 0x9041,
    MIPS16      = 0x266,
    MIPSFPU     = 0x366,
    MIPSFPU16   = 0x466,
    PowerPC     = 0x1f0,
    PowerPCFP   = 0x1f1,
    R4000       = 0x166,
    RISCV32     = 0x5032,
    RISCV64     = 0x5064,
    SH3         = 0x1a2,
    SH3DSP      = 0x1a3,
    SH4         = 0x1a4,
    SH5         = 0x1a8,
    Thumb       = 0x1c2,
    WCEMIPSV2   = 0x169,
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#characteristics
enum class Characteristics : uint16_t
{
    RelocsStripped       = 0x0001,
    ExecutableImage      = 0x0002,
    FileLineNumsStripped = 0x0004,
    AgressiveWSTrim      = 0x0008, // Obsolete
    LargeAddressAware    = 0x0020,
    BytesReversedLO      = 0x0080, // Obsolete
    Machine32            = 0x0100,
    DebugStripped        = 0x0200,
    RemovableRunFromSwap = 0x0400,
    NetRunFromSwap       = 0x0800,
    System               = 0x1000,
    DLL                  = 0x2000,
    UpSystemOnly         = 0x4000,
    BytesReversedHi      = 0x8000, // Obsolete

    // 0x0040 reserved for future use
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#coff-file-header-object-and-image
struct COFFHeader
{
    MachineType machine_type;
    uint16_t section_count;
    uint32_t time_date_stamp;
    uint32_t symbol_table_offset;
    uint32_t symbol_count;
    uint16_t optional_header_size;
    Characteristics characteristics;
};

enum class OptionalHeaderMagic : uint16_t
{
    PE32     = 0x10B,
    ROM      = 0x107,
    PE32Plus = 0x20B,
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#optional-header-standard-fields-image-only
struct OptionalHeader
{
    OptionalHeaderMagic magic;
    struct
    {
        uint8_t major;
        uint8_t minor;
    } linker_version;
    uint32_t code_size;
    uint32_t initialized_data_size;
    uint32_t uninitialized_data_size;
    uint32_t entry_point_offset;
    uint32_t base_address;
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#windows-subsystem
enum class WindowsSubsystem : uint16_t
{
    Unknown                = 0,
    Native                 = 1,
    GUI                    = 2,
    CUI                    = 3,
    OS2CUI                 = 5,
    POSIXCUI               = 7,
    NativeWindows          = 8,
    WindowsCEGUI           = 9,
    EFIApplication         = 10,
    EFIBootServiceDriver   = 11,
    EFIRuntimeDriver       = 12,
    EFIROM                 = 13,
    XBOX                   = 14,
    WindowsBootApplication = 16,
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#dll-characteristics
enum class DLLCharacteristics : uint16_t
{
    HighEntropyVA       = 0x0020,
    DynamicBase         = 0x0040,
    ForceIntegrity      = 0x0080,
    NXCompat            = 0x0100,
    NoIsolation         = 0x200,
    NoSEH               = 0x400,
    NoBind              = 0x800,
    AppContainer        = 0x1000,
    WDMDriver           = 0x2000,
    GuardCF             = 0x4000,
    TerminalServerAware = 0x8000,
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#optional-header-windows-specific-fields-image-only
struct OptionalWindowsHeader32
{
    uint32_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } os_version;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } image_version;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } subsystem_version;
    uint32_t win32_version; // Must be 0
    uint32_t image_size;
    uint32_t header_size;
    uint32_t checksum;
    WindowsSubsystem subsystem;
    DLLCharacteristics dll_characteristics;
    uint32_t stack_reserve_size;
    uint32_t stack_commit_size;
    uint32_t heap_reserve_size;
    uint32_t heap_commit_size;
    uint32_t loader_flags; // Must be 0
    uint32_t rva_and_size_count;
};

struct OptionalWindowsHeader32Plus
{
    uint64_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } os_version;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } image_version;
    struct
    {
        uint16_t major;
        uint16_t minor;
    } subsystem_version;
    uint32_t win32_version; // Must be 0
    uint32_t image_size;
    uint32_t header_size;
    uint32_t checksum;
    WindowsSubsystem subsystem;
    DLLCharacteristics dll_characteristics;
    uint64_t stack_reserve_size;
    uint64_t stack_commit_size;
    uint64_t heap_reserve_size;
    uint64_t heap_commit_size;
    uint32_t loader_flags; // Must be 0
    uint32_t rva_and_size_count;
};

struct ImageDataDirectory
{
    uint32_t rva;
    uint32_t size;
};

enum class DataDirectoryType
{
    Export,
    Import,
    Resource,
    Exception,
    Certificate,
    BaseRelocation,
    Debug,
    Architecture, // Reserved
    GlobalPtr,
    TLS,
    LoadConfig,
    BoundImport,
    IAT,
    DelayImportDescriptor,
    CLRRuntimeHeader,
    Reserved,
    COUNT
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#section-flags
enum class SectionFlags : uint32_t
{
    NoPad                     = 0x8,
    ContainsCode              = 0x20,
    ContainsInitializedData   = 0x40,
    ContainsUninitializedData = 0x80,
    LinkInfo                  = 0x200,
    LinkRemove                = 0x800,
    LinkCOMDAT                = 0x1000,
    GlobalPointerRelative     = 0x8000,
    Align1Byte                = 0x100000,
    Align2Bytes               = 0x200000,
    Align4Bytes               = 0x300000,
    Align8Bytes               = 0x400000,
    Align16Bytes              = 0x500000,
    Align32Bytes              = 0x600000,
    Align64Bytes              = 0x700000,
    Align128Bytes             = 0x800000,
    Align256Bytes             = 0x900000,
    Align512Bytes             = 0xa00000,
    Align1024Bytes            = 0xb00000,
    Align2048Bytes            = 0xc00000,
    Align4096Bytes            = 0xd00000,
    Align8192Bytes            = 0xe00000,
    LinkExtendedRelocations   = 0x1000000,
    MemDiscardable            = 0x2000000,
    MemNotCached              = 0x4000000,
    MemNotPaged               = 0x8000000,
    MemShared                 = 0x10000000,
    MemExecute                = 0x20000000,
    MemRead                   = 0x40000000,
    MemWrite                  = 0x80000000,
};

enum class SectionType
{
    Debug,
    Drectve,
    Edata,
    Idata,
    Pdata,
    Reloc,
    TLS,
    Rsrc,
    Cormeta,
    SXData,
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#section-table-section-headers
struct SectionHeader
{
    // 8-byte null-padded UTF-8-encoded string.
    char const name[8];
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t raw_data_size;
    uint32_t raw_data_offset;
    uint32_t relocations_offset;
    uint32_t line_numbers_offset;
    uint16_t relocation_count;
    uint16_t line_number_count;
    SectionFlags flags;
};

// https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#import-directory-table
struct ImportDirectoryEntry
{
    uint32_t lookup_table_rva;
    uint32_t time_date_stamp;
    uint32_t forward_chain;
    uint32_t name_rva;
    uint32_t iat_rva;
};

class PE
{
public:
    bool load(char const* data, bool writable);

    uint32_t directory_count() const;

    void examine_imports();

    // Scan the import directory to ensure all dlls requested are present. Then,
    // resort the directory entries, inserting the requested dlls in front.
    bool escalate(std::vector<std::string> const& dlls, char* out_data);

private:
    std::vector<ImportDirectoryEntry> extract_imports(uint32_t& file_offset);

    // Translates an RVA to a file offset
    uint32_t resolve_rva(uint32_t rva);

    char const* data_;
    bool valid_ = false;

    COFFHeader const* header_                             = nullptr;
    OptionalHeader const* optional_header_                = nullptr;
    OptionalWindowsHeader32 const* win32_header_          = nullptr;
    OptionalWindowsHeader32Plus const* win32_plus_header_ = nullptr;
    ImageDataDirectory const* directories[(int)DataDirectoryType::COUNT] = {};
    std::vector<SectionHeader const*> section_headers_;
    std::unordered_map<SectionType, SectionHeader const*> section_index_;
};
