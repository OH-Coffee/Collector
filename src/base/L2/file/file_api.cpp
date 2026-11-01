
#include <filesystem>
#include <fstream>
#include <limits.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "file/file_api.hpp"
#include "string/string_api.hpp"


std::string file_dir(const std::string &i_path)
{
    return std::filesystem::path(i_path).parent_path();
}
std::string file_name(const std::string &i_path)
{
    return std::filesystem::path(i_path).filename();
}

std::string file_realpath(const std::string &i_path)
{
    char buf[PATH_MAX] = {0};
    return (nullptr == realpath(i_path.c_str(), buf)) ? "" : buf;
}

size_t file_size(const std::string& i_path)
{
    struct stat fs{};

    return (lstat(i_path.c_str(), &fs) == 0) ? fs.st_size : 0;
}

ret_code file_is_exist(const std::string &i_path)
{
    return ( (access(i_path.c_str(), F_OK) == 0) ? RC_OK : RC_FILE_NO_EXIST );
}

ret_code file_create(const std::string &i_path)
{
    ret_code r_code = RC_UNKNOWN;
    
    std::error_code ec;
    do {
        std::filesystem::path path(i_path);
        if(path.empty())       { break; }
        if(path.is_relative()) { break; }
        
        // 路径已存在，不需要创建路径，认为成功。
        if(std::filesystem::exists(path, ec)) {
            r_code = RC_OK; break;
        }
        
        // 路径不存在，需要创建。提取目录部分，先创建目录。
        std::filesystem::path dir(path.parent_path());
        if(!std::filesystem::exists(dir, ec)) {
            std::filesystem::create_directories(dir, ec);
            
            if(!std::filesystem::exists(dir, ec)) {
                r_code = RC_FAIL; break;
            }
        }
        
        std::ofstream file(path);
        if(file.is_open()) {
            r_code = RC_OK;
        } else {
            r_code = RC_FAIL;
        }
        file.close();
    } while (false);
    
    return r_code;
}


ret_code file_remove(const std::string &i_path)
{
    ret_code r_code = RC_UNKNOWN;
    
    std::error_code ec;
    do {
        if("/" == std::filesystem::absolute(i_path)) { break; }

        std::filesystem::path path(i_path);
        if(path.empty())       { r_code = RC_ARG_ERR; break; }
        if(path.is_relative()) { r_code = RC_ARG_ERR; break; }
        
        if(!std::filesystem::exists(path, ec)) {
            r_code = RC_OK; break;
        }
        
        uintmax_t r_count = std::filesystem::remove_all(i_path, ec);
        if(r_count != static_cast<std::uintmax_t>(-1)) {
            r_code = RC_OK;
        } else {
            r_code = RC_FILE_REMOVE_ERR;
        }
    } while (false);
    
    return r_code;
}

size_t file_read(const std::string &i_path, const size_t i_buf_len, char *o_buf)
{
    size_t r_size = 0;

    do {
        if(i_path.empty())                     { break; }
        if(i_buf_len <= 0 || nullptr == o_buf) { break; }

        FileStreamPtr fs(fopen(i_path.c_str(), "rb"));
        if(nullptr != fs) {
            r_size = fread(o_buf, 1, i_buf_len, fs.get());
        }
    } while (false);

    return r_size;
}

std::string file_read_all_s(const std::string &i_path, uint32_t i_max)
{
    const auto data = file_read_all_v(i_path, (uint32_t)i_max);

    return std::string{ std::move_iterator{data.begin()}, std::move_iterator{data.end()} };
}

std::vector<uint8_t> file_read_all_v(const std::string &i_path, uint32_t i_max)
{
    std::vector<uint8_t> ret_value;

    do {
        FileStreamPtr fs(fopen(i_path.c_str(), "rb"));
        if(nullptr == fs) { break; }

        const size_t buf_len = 1024;
        char         buf[buf_len + 1]; buf[buf_len] = '\0';

        for(uintmax_t i = 0; i < (i_max / buf_len) + 1; i++) {
            bzero(buf, buf_len);

            auto r_read = fread(buf, 1, buf_len, fs.get());
            ret_value.insert(ret_value.end(), buf, buf + r_read);

            if(r_read < buf_len) { break; }
        }
    } while (false);

    return ret_value;
}

size_t file_write(const std::string &i_path, const std::string &i_buf)
{
    std::ofstream fs(i_path);

    if(fs.is_open()) {
        fs << i_buf; return fs.tellp();
    }

    return 0;
}

size_t file_write(const std::string &i_path, const std::vector<uint8_t> &i_buf)
{
    size_t ret_size = -1;

    do {
        FileStreamPtr fs(fopen(i_path.c_str(), "w+"));
        if(nullptr == fs) { break; }

        size_t io_size   = 0;
        size_t remainder = i_buf.size();

        while((remainder -= io_size) > 0) {
            io_size = ::fwrite(&i_buf[i_buf.size() - remainder], sizeof(i_buf[0]), remainder, fs.get());
        }
    } while (false);

    return ret_size;
}

size_t file_append(const std::string &i_path, const std::string &i_buf)
{
    std::ofstream fs(i_path, std::ios::app);

    if(fs.is_open()) {
        fs << i_buf; return fs.tellp();
    }

    return 0;
}

size_t file_append(const std::string &i_path, const std::vector<uint8_t> &i_buf)
{
    size_t ret_size = 0;

    do {
        FileStreamPtr fs(fopen(i_path.c_str(), "a"));
        if(nullptr == fs) { break; }

        size_t io_size   = 0;
        size_t remainder = i_buf.size();

        while(remainder > 0) {
            io_size = ::fwrite(&i_buf[i_buf.size() - remainder], sizeof(i_buf[0]), remainder, fs.get());
            if(io_size == 0) { break; }
            remainder -= io_size;
            ret_size += io_size;
        }
    } while (false);

    return ret_size;
}

void file_read_lines(const std::string &i_path, const std::function<bool(const std::string &)> &i_cb)
{
    std::ifstream fs(i_path);

    if(fs.is_open()) {
        std::string line;
        while(std::getline(fs, line)) {
            if(!i_cb(line)) { break; }
        }
    }
}

std::vector<std::string> file_read_lines(const std::string &i_path, size_t i_max_line)
{
    std::vector<std::string> ret_lines;

    do {
        std::ifstream fs(i_path);
        if(!fs.is_open()) { break; }

        std::string line;
        for(decltype(i_max_line) i = 0; i < i_max_line; i++) {
            if(!std::getline(fs, line)) { break; }

            ret_lines.push_back(line);
        }
    } while (false);

    return ret_lines;
}

ret_code file_lock(int i_fd)
{
    ret_code r_code = RC_UNKNOWN;
    
    if(i_fd >= 0) {
        r_code = ( (0 == flock(i_fd, LOCK_EX | LOCK_NB)) ? RC_OK : RC_FILE_FLOCK_ERR );
    } else {
        r_code = RC_FILE_FD_NULL;
    }
    
    return r_code;
}

ret_code file_lock(const char *i_path, int &o_fd)
{
    ret_code r_code = RC_UNKNOWN;
        
    do {
        if(o_fd > 0) { r_code = RC_FAIL; break; }
        
        o_fd = open(i_path, O_CREAT | O_RDONLY | O_CLOEXEC, S_IRWXU);
        if(o_fd < 0) { r_code = RC_FILE_OPEN_ERR; break; }
        
        r_code = file_lock(o_fd);
    } while (false);
    
    return r_code;
}

ret_code file_unlock(int i_fd)
{
    ret_code r_code = RC_UNKNOWN;
        
    if(i_fd >= 0) {
        r_code = ( (0 == flock(i_fd, LOCK_UN | LOCK_NB)) ? RC_OK : RC_FILE_FLOCK_ERR );
    } else {
        r_code = RC_FILE_FD_NULL;
    }
    
    return r_code;
}

ret_code file_is_lock(int i_fd)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(i_fd < 0) { r_code = RC_FILE_FD_NULL; break; }
        
        if(0 == flock(i_fd, LOCK_EX | LOCK_NB)) {
            flock(i_fd, LOCK_UN);
        } else {
            r_code = RC_OK; // 获取锁失败，说明文件已经上锁。
        }
    } while (false);
    
    return r_code;
}

std::string path_traceback(const std::string &i_path)
{
    size_t site = i_path.rfind('/');
    if(std::string::npos != site && site <= i_path.size()) {
        return i_path.substr(0, site);
    }

    return "";
}
std::string path_traceback_find(std::string i_start_path, const std::string &i_dst_name)
{
    std::string path = i_start_path + "/" + i_dst_name;
    
    while(RC_OK != file_is_exist(path)) {
        i_start_path = path_traceback(i_start_path);
        
        if(i_start_path.empty()) {
            return "";
        } else {
            path.assign(i_start_path).append("/").append(i_dst_name);
        }
    }
    
    return path;
}

ret_code file_save_num(int32_t i_num, const char *i_file)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        r_code = file_create(i_file);
        if(RC_OK != r_code) { break; }

        FileStreamPtr fs(fopen(i_file, "w"));
        if(fs == nullptr) {
            r_code = RC_FILE_OPEN_ERR; break;
        }
        
        std::string num_str = std::to_string(i_num);
        size_t r_write = fwrite(num_str.c_str(), num_str.size(), 1, fs.get());
        
        r_code = (r_write > 0 ? RC_OK : RC_FILE_WRITE_ERR);
    } while (false);
    
    return r_code;
}

int64_t file_read_num(const std::string &i_file)
{
    int64_t r_num = 0;

    do {
        FileStreamPtr fs(fopen(i_file.c_str(), "r"));
        if(nullptr == fs) {
            break;
        }
        
        char num_str[64]; bzero(num_str, sizeof(num_str));
        
        if(size_t r_read = fread(num_str, 1, sizeof(num_str), fs.get()); r_read > 0) {
            r_num  = string_to_integer<int64_t>(num_str);
        }
    } while (false);
    
    return r_num;
}

ret_code file_read_num(const char *i_file, int32_t &o_num)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        FileStreamPtr fs(fopen(i_file, "r"));
        if(nullptr == fs) {
            r_code = RC_FILE_OPEN_ERR; break;
        }
        
        char num_str[32] = {0};
        size_t r_read = fread(num_str, 1, 32, fs.get());
        
        o_num  = string_to_integer<int32_t>(num_str);
        r_code = (r_read > 0 ? RC_OK : RC_FILE_READ_ERR);
    } while (false);
    
    return r_code;
}

std::vector<std::string> file_names_in_dir(const std::string &i_dir)
{
    std::vector<std::string> ret_names;

    std::error_code ec;
    for (const auto &item : std::filesystem::directory_iterator(i_dir, ec)) {
        ret_names.push_back(item.path().filename().string());
    }

    return ret_names;
}


const char* path_suffix(const char *i_path)
{
    const char *ret_suffix = nullptr;
    
    do {
        if(nullptr == i_path) { break; }
        
        const char *ptr = strrchr(i_path, '/');
        if(nullptr == ptr) { break; }
        
        const char *pos = strrchr((ptr != nullptr ? ptr : i_path), '.');
        if(nullptr == pos) { break; }
        
        ret_suffix = (strlen(i_path) != (size_t)(pos - i_path + 1)) ? pos : nullptr;
    } while (false);
    
    return ret_suffix;
}

std::string path_reverse(const std::string &i_path)
{
    return string_reverse(i_path, "/");
}
