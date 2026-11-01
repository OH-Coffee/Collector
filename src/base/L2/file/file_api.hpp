
#ifndef file_api_hpp
#define file_api_hpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

#include "pch.hpp"

struct FileStreamCloser     { void operator()(FILE *i_fs) const { if(i_fs != nullptr) { fclose(i_fs); } } };
struct FileDescriptorCloser { void operator()(int   i_fd) const { if(i_fd >= 0)       {  close(i_fd); } } };

using FileStreamPtr     = std::unique_ptr<FILE, FileStreamCloser>;
using FileDescriptorPtr = std::unique_ptr<int,  FileDescriptorCloser>;

typedef enum : uint64_t {
    FSI_TYPE_UNKNOWN = 0, // FSI：File Sign Info
    FSI_TYPE_SIGN_ID = (uint64_t)0x01 << 0,
    FSI_TYPE_TEAM_ID = (uint64_t)0x01 << 1,
    FSI_TYPE_ALL     = ~(uint64_t)0,
} FileSignInfoType;

typedef std::string                             Path;
typedef std::vector<std::string>                Paths;
typedef std::vector<std::string>                FileLines;
typedef std::map<FileSignInfoType, std::string> FileSignInfo;


std::string file_dir(const std::string &i_path);
std::string file_name(const std::string &i_path);
std::string file_realpath(const std::string &i_path);

size_t      file_size(const std::string &i_path);

ret_code    file_is_exist(const std::string &i_path);
ret_code    file_create(const std::string &i_path);
ret_code    file_remove(const std::string &i_path);

size_t                  file_read(const std::string &i_path, size_t i_buf_len, char *o_buf);
std::string             file_read_all_s(const std::string &i_path, uint32_t i_max = 1024 * 1024 * 5);
std::vector<uint8_t>    file_read_all_v(const std::string &i_path, uint32_t i_max = 1024 * 1024 * 5);

size_t      file_write(const std::string &i_path, const std::string &i_buf);
size_t      file_write(const std::string &i_path, const std::vector<uint8_t> &i_buf);

size_t      file_append(const std::string &i_path, const std::string &i_buf);
size_t      file_append(const std::string &i_path, const std::vector<uint8_t> &i_buf);

void                        file_read_lines(const std::string &i_path, const std::function<bool(const std::string &)> &i_cb);
std::vector<std::string>    file_read_lines(const std::string &i_path, size_t i_max = 10000);

ret_code file_lock(int i_fd);
ret_code file_lock(const char *i_path, int &o_fd);
ret_code file_unlock(int i_fd);
ret_code file_is_lock(int i_fd);

std::string path_traceback(const std::string &i_path);
std::string path_traceback_find(std::string i_start_path, const std::string &i_dst_name);

ret_code file_save_num(int32_t i_num, const char *i_file);
int64_t  file_read_num(const std::string &i_file);
ret_code file_read_num(const char *i_file, int32_t &o_num);

std::vector<std::string> file_names_in_dir(const std::string &i_dir);

const char* path_suffix(const char *i_path);
std::string path_reverse(const std::string &i_path);

FileSignInfo file_sign_info(const std::string &i_path, FileSignInfoType i_types = FSI_TYPE_ALL);

#endif /* file_api_hpp */
