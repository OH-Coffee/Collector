
#ifndef file_file_type_hpp
#define file_file_type_hpp

#include <cstdint>
#include <string>

typedef enum : uint16_t
{
    FT_BASIC_UNKNOWN = 0,
    FT_BASIC_BLOCK,
    FT_BASIC_CHARACTER,
    FT_BASIC_DIR,
    FT_BASIC_FIFO,
    FT_BASIC_LINK,
    FT_BASIC_REGULAR,
    FT_BASIC_SOCKET,
} FileBasicType;

typedef enum : uint16_t
{
    FT_BINARY_UNKNOWN = 0,
    FT_BINARY_ELF_REL,
    FT_BINARY_ELF_EXEC,
    FT_BINARY_ELF_DYN,
    FT_BINARY_ELF_CORE,
} FileBinaryType;

typedef enum : uint16_t
{
    FT_SCRIPT_UNKNOWN = 0,
} FileScriptType;

typedef enum : uint16_t
{
    FT_SUFFIX_UNKNOWN = 0,
} FileSuffixType;

typedef struct FileType
{
    FileBasicType   basic_t   { FT_BASIC_UNKNOWN  };
    FileBinaryType  binary_t  { FT_BINARY_UNKNOWN };
    FileScriptType  script_t  { FT_SCRIPT_UNKNOWN };
    FileSuffixType  suffix_t  { FT_SUFFIX_UNKNOWN };
} FileType;

typedef enum : uint16_t
{
    FT_CLASS_UNKNOWN = (uint16_t)(0),
    FT_CLASS_BASIC   = (uint16_t)(0x01 << 0),
    FT_CLASS_BINARY  = (uint16_t)(0x01 << 1),
    FT_CLASS_SCRIPT  = (uint16_t)(0x01 << 2),
    FT_CLASS_SUFFIX  = (uint16_t)(0x01 << 3),
    FT_CLASS_ALL     = (uint16_t)(~0),
} FileTypeClass;

FileType file_type(const std::string &i_path, FileTypeClass = FT_CLASS_BASIC);

#endif // file_file_type_hpp
