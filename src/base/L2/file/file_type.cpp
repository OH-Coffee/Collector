
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <strings.h>

#if defined(OS_LINUX)
    #include <elf.h>
#endif
#include <sys/stat.h>

#include "file/file_type.hpp"


FileBasicType  file_basic_t(const std::string &i_path);
FileBinaryType file_binary_t(const char *i_buf, size_t len);
FileScriptType file_script_t(const char *i_buf, size_t len);
FileSuffixType file_suffix_t(const std::string &i_path);

FileType file_type(const std::string &i_path, FileTypeClass i_class)
{
    FileType ret_type;

    FILE *fs = nullptr;
    do {
        FileBasicType basic_type = file_basic_t(i_path);

        ret_type.basic_t  = (i_class & FT_CLASS_BASIC)  ? basic_type            : FT_BASIC_UNKNOWN;
        ret_type.suffix_t = (i_class & FT_CLASS_SUFFIX) ? file_suffix_t(i_path) : FT_SUFFIX_UNKNOWN;

        bool set_binary_flag = (i_class & FT_CLASS_BINARY);
        bool set_script_flag = (i_class & FT_CLASS_SUFFIX);
        bool need_read       = set_binary_flag || set_script_flag;

        if((need_read) && (FT_BASIC_REGULAR == basic_type) && (nullptr != (fs = fopen(i_path.c_str(), "rb")))) {
            constexpr size_t header_len = 32;
            char header_buf[header_len];
            bzero(header_buf, header_len);

            auto r_read = fread(header_buf, 1, header_len, fs);
            if(r_read <= 0) { break; }

            ret_type.binary_t = set_binary_flag ? file_binary_t(header_buf, header_len) : FT_BINARY_UNKNOWN;
            ret_type.script_t = set_script_flag ? file_script_t(header_buf, header_len) : FT_SCRIPT_UNKNOWN;
        }
    } while (false);
    if(nullptr != fs) { fclose(fs); fs = nullptr; }

    return ret_type;
}

FileBasicType file_basic_t(const std::string &i_path)
{
    FileBasicType ret_type = FT_BASIC_UNKNOWN;

    do {
        using namespace std::filesystem;

        std::error_code         ec;
        std::filesystem::path   fs_path(i_path);

#define FILE_TYPE_IS(i_func, i_type) if( (i_func)(fs_path, ec) ) { ret_type = (i_type); break; }
        FILE_TYPE_IS(is_block_file,     FT_BASIC_BLOCK)
        FILE_TYPE_IS(is_character_file, FT_BASIC_CHARACTER)
        FILE_TYPE_IS(is_directory,      FT_BASIC_DIR)
        FILE_TYPE_IS(is_fifo,           FT_BASIC_FIFO)
        FILE_TYPE_IS(is_symlink,        FT_BASIC_LINK)
        FILE_TYPE_IS(is_regular_file,   FT_BASIC_REGULAR)
        FILE_TYPE_IS(is_socket,         FT_BASIC_SOCKET)
#undef  FILE_TYPE_IS
    } while(false);

    return ret_type;
}

static FileBinaryType file_binary_t_for_elf(const char *i_buf, const size_t i_len)
{
#if defined(OS_LINUX)
    static const char elf_header_magic[] = { 0x7F, 'E', 'L', 'F' };

    FileBinaryType ret_type = FT_BINARY_UNKNOWN;

    do {
        if(i_len < (EI_NIDENT + sizeof(uint16_t)))   { break; }
        if(0 != strncmp(i_buf, elf_header_magic, 4)) { break; }

        uint16_t elf_type = *( (uint16_t *)(i_buf + EI_NIDENT) );
        switch (elf_type) {
            case ET_REL:  { ret_type = FT_BINARY_ELF_REL;   } break;
            case ET_EXEC: { ret_type = FT_BINARY_ELF_EXEC;  } break;
            case ET_DYN:  { ret_type = FT_BINARY_ELF_DYN;   } break;
            case ET_CORE: { ret_type = FT_BINARY_ELF_CORE;  } break;
            default: { } break;
        }
    } while (false);

    return ret_type;
#endif
    return FT_BINARY_UNKNOWN;
}

FileBinaryType file_binary_t(const char *i_buf, size_t i_len)
{
    return file_binary_t_for_elf(i_buf, i_len);
}

FileScriptType file_script_t(const char *i_buf, size_t len)
{
    return FT_SCRIPT_UNKNOWN;
}

FileSuffixType file_suffix_t(const std::string &i_path)
{
    return FT_SUFFIX_UNKNOWN;
}
