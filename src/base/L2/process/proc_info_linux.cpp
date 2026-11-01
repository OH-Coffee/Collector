
#include <cerrno>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file/file_type.hpp"
#include "string/string_api.hpp"
#include "string/string_split.hpp"
#include "file/file_api.hpp"
#include "process/proc_info.hpp"

#ifndef HZ
    #if defined(_SC_CLK_TCK) && (!defined(OPENSSL_SYS_VMS) || __CTRL_VER >= 70000000)
        #define HZ ((double)sysconf(_SC_CLK_TCK))
    #else
        #ifndef CLK_TCK
            #ifndef _BSD_CLK_TCK_ /* FreeBSD hack */
                #define HZ  100.0
            #else /* _BSD_CLK_TCK_ */
                #define HZ ((double)_BSD_CLK_TCK_)
            #endif
        #else /* CLK_TCK */
            #define HZ ((double)CLK_TCK)
        #endif
    #endif
#endif

char * proc_readlink(const std::string &i_path, uint32_t i_buf_len, char *o_buf)
{
    char *r_data = nullptr;

    do {
        if(i_path.empty() || nullptr == o_buf || i_buf_len <= 0) { break; }

        if(readlink(i_path.c_str(), o_buf, i_buf_len) > 0) {
            r_data = o_buf;
        }
    } while (false);

    return r_data;
}

std::string proc_readlink(const std::string &i_path)
{
    char buf[PATH_MAX] = { 0 }; return proc_readlink(i_path, PATH_MAX, buf) ? buf : "";
}


// 用于读取/proc/PID/cmdline等文件；这类文件以'\0'作为分隔符，需要特殊处理，才能保证字符串的完整
std::string proc_read_all(const std::string &i_path, char i_replacement, size_t i_max)
{
    std::string r_string;

    FILE *fs = fopen(i_path.c_str(), "r");
    do {
        if(nullptr == fs) { break; }

        const size_t buf_len = 1024;
        char         buf[buf_len + 1]; buf[buf_len] = '\0';

        for(size_t i = 0; i < (i_max / buf_len) + 1; i++) {
            bzero(buf, buf_len);

            auto r_read = fread(buf, 1, buf_len, fs);
            for(size_t j = 0; j < r_read; j ++) {
                if('\0' == buf[j]) { buf[j] = i_replacement; }
            }
            r_string.append(buf);

            if(r_read < buf_len) { break; }
        }
        if(!r_string.empty() && i_replacement == r_string.back()) { r_string.pop_back(); }
    } while (false);
    if(nullptr != fs) { fclose(fs); fs = nullptr; }

    return r_string;
}

// 用于读取/proc/<PID>/stat文件
std::vector<std::string> proc_stat(pid_t i_pid)
{
    std::vector<std::string> r_stat;

    do {
        std::string text = file_read_all_s("/proc/" + std::to_string(i_pid) + "/stat");
        if(text.empty()) { break; }

        auto pos_s = text.find('(');   // 'pos_s' means 'position of start'
        auto pos_e = text.find(')');   // 'pos_e' means 'position of end'
        if(std::string::npos == pos_s || std::string::npos == pos_e) { break; }

        r_stat.push_back(text.substr(0, pos_s - 1));                    // pid
        r_stat.push_back(text.substr(pos_s + 1, pos_e - pos_s - 1));    // name

        if('\n' == text.back()) { text.pop_back(); }

        // 获取余下字段；'pos_e + 2'是为了跳过')'以及其后的空格
        auto fields = string_split<std::vector<std::string>>(&text[pos_e + 2], " ");
        r_stat.insert(r_stat.end(), fields.begin(), fields.end());
    } while(false);

    return r_stat;
}

// 用于读取/proc/<PID>/status文件，并将其分割为键值对
std::map<std::string, std::string> proc_status(pid_t i_pid)
{
    std::map<std::string, std::string> r_status;

    do {
        std::string path = "/proc/" + std::to_string(i_pid) + "/status";
        if(0 != access(path.c_str(), F_OK)) { break; }

        static const std::string delim(":\t");

        for(const auto &it : string_split<std::list<std::string>>(file_read_all_s(path), "\n")) {
            auto site = it.find(delim);
            if(site != std::string::npos) {
                r_status.insert( { it.substr(0, site), it.substr(site + delim.size()) } );
            }
        }
    } while (false);

    return r_status;
}

int proc_all_pids(pid_t o_pid_list[], int i_list_size)
{
    int r_counter = 0;

    DIR *dir = opendir("/proc");
    do {
        if(nullptr == dir) { break; }

        struct dirent *dir_ptr = nullptr;
        while (nullptr != (dir_ptr = readdir(dir)) && r_counter < i_list_size) {
            if(0 == (dir_ptr->d_type & DT_DIR)) { continue; }

            if(string_is_digit(dir_ptr->d_name, strlen(dir_ptr->d_name))) {
                o_pid_list[r_counter++] = (pid_t)strtol(dir_ptr->d_name, nullptr, 0);
            }
        }
    } while(false);
    if(nullptr != dir) { closedir(dir); dir = nullptr; }

    return r_counter;
}

int proc_pids_by_name(const char *i_name, std::vector<pid_t> &o_pids, bool i_ignore_case)
{
    int ret_counter = 0;
    
    do {
        pid_t pids[5000] = { 0 };

        int pid_num = proc_all_pids(pids, sizeof(pids) / sizeof(pids[0]));
        if(pid_num <= 0) { break; }

        const size_t len = strlen(i_name);

        for(int index = 0; index < pid_num; ++index) {
            const pid_t pid = pids[index];
            bool match = false;
            if(!i_ignore_case) {
                if(0 == strncmp(proc_name(pid).c_str(), i_name, len))            { match = true; }
            } else {
                if(0 == strncasecmp(proc_name(pid).c_str(), i_name, len)) { match = true; }
            }
            if(match) { o_pids.push_back(pid); ++ret_counter; }
        }
    } while(false);
    
    return ret_counter;
}

int proc_pids_by_path(const char *i_path, std::vector<pid_t> &o_pids, bool i_ignore_case)
{
    int ret_counter = 0;

    do {
        pid_t pids[5000] = { 0 };

        int pid_num = proc_all_pids(pids, sizeof(pids) / sizeof(pids[0]));
        if(pid_num <= 0) { break; }

        const size_t len = strlen(i_path);

        for(int index = 0; index < pid_num; ++index) {
            const pid_t pid = pids[index];
            bool match = false;
            if(!i_ignore_case) {
                if(0 == strncmp(proc_path(pid).c_str(), i_path, len))            { match = true; }
            } else {
                if(0 == strncasecmp(proc_path(pid).c_str(), i_path, len)) { match = true; }
            }
            if(match) { o_pids.push_back(pid); ++ret_counter; }
        }
    } while(false);

    return ret_counter;
}

char* proc_name(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    if(i_buf_len <= 0)   { return nullptr; }
    if(nullptr == o_buf) { return nullptr; }

    std::string text = file_read_all_s("/proc/" + std::to_string(i_pid) + "/comm");
    if(!text.empty() && '\n' == text.back()) { text.pop_back(); }

    if(!text.empty()) {
        strncpy(o_buf, text.c_str(), (text.size() > i_buf_len ? i_buf_len : text.size()));
    }

    return o_buf;
}

std::string proc_name(pid_t i_pid)
{
    char buf[NAME_MAX]; bzero(buf, NAME_MAX);

    if(nullptr != proc_name(i_pid, NAME_MAX, buf)) {
        return buf;
    }

    return "";
}


char * proc_path(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    return proc_readlink("/proc/" + std::to_string(i_pid) + "/exe", i_buf_len, o_buf);
}

std::string proc_path(pid_t i_pid)
{
    char buf[PATH_MAX] = { 0 }; return proc_path(i_pid, PATH_MAX, buf) ? buf : "";
}

char * proc_work_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    return proc_readlink("/proc/" + std::to_string(i_pid) + "/cwd", i_buf_len, o_buf);
}

std::string proc_work_dir(pid_t i_pid)
{
    char buf[PATH_MAX] = { 0 }; return proc_work_dir(i_pid, PATH_MAX, buf) ? buf : "";
}

char * proc_root_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    return proc_readlink("/proc/" + std::to_string(i_pid) + "/root", i_buf_len, o_buf);
}

std::string proc_root_dir(pid_t i_pid)
{
    char buf[PATH_MAX] = { 0 }; return proc_root_dir(i_pid, PATH_MAX, buf) ? buf : "";
}

std::vector<std::string> proc_args_info(pid_t i_pid, ProcArgsInfo i_type)
{
    std::vector<std::string> r_args;

    do {
        const char  delim = 1;
        std::string pid   = std::to_string(i_pid);
        std::string path  = "/proc/" + pid + "/cmdline";

        auto argv = string_split<std::vector<std::string>>(proc_read_all(path, delim), std::string(&delim, 1));
        if(argv.empty()) { break; }

        if(i_type & PROC_ARG_ARGC) {
            r_args.push_back(std::to_string(argv.size()));
        }

        if(i_type & PROC_ARG_PATH) {
            r_args.push_back(proc_path(i_pid));
        }

        if(i_type & PROC_ARG_ARGV) {
            r_args.insert(r_args.end(), argv.begin(), argv.end());
        }

        if(i_type & PROC_ARG_ENVS) {
            path = "/proc/" + pid + "/environ";
            auto envs = string_split<std::vector<std::string>>(proc_read_all(path, delim), std::string(&delim, 1));
            r_args.insert(r_args.end(), envs.begin(), envs.end());
        }
    } while (false);

    return r_args;
}

std::string proc_args(pid_t i_pid)
{
    return proc_cmdline(i_pid);
}

std::string proc_envs(pid_t i_pid)
{
    return proc_read_all("/proc/" + std::to_string(i_pid) + "/environ");
}

std::string proc_cmdline(pid_t i_pid)
{
    return proc_read_all("/proc/" + std::to_string(i_pid) + "/cmdline");
}

bool proc_threads(pid_t i_pid, std::vector<uint64_t> &o_tid)
{
    int r_counter = 0;

    DIR *dir = opendir(std::string("/proc/" + std::to_string(i_pid) + "/task/").c_str());
    do {
        if(nullptr == dir) { break; }

        struct dirent *dir_ptr = nullptr;
        while (nullptr != (dir_ptr = readdir(dir))) {
            if(0 == (dir_ptr->d_type & DT_DIR)) { continue; }

            if(string_is_digit(dir_ptr->d_name, strlen(dir_ptr->d_name))) {
                o_tid.push_back((uint64_t)strtol(dir_ptr->d_name, nullptr, 0));
                ++r_counter;
            }
        }
    } while(false);
    if(nullptr != dir) { closedir(dir); dir = nullptr; }

    return r_counter;
}

bool proc_threads_cpu(pid_t i_pid, std::map<uint64_t, int> &o_info)
{
    return false;
}

uint64_t proc_cpu_time(pid_t i_pid)
{
    uint64_t r_value = 0;
    
    do {
        static const uint64_t clock_tick = HZ;

        auto process_stat = proc_stat(i_pid);
        if(process_stat.size() < PROC_STAT_START_TIME) {
            break;
        }

        uint64_t usr_time = strtol(process_stat[PROC_STAT_USER_TIME].c_str(),  nullptr, 0);
        uint64_t sys_time = strtol(process_stat[PROC_STAT_SYS_TIME].c_str(),   nullptr, 0);

        r_value = (usr_time + sys_time) * 1000 / clock_tick; // 转换为毫秒
    } while (false);
    
    return r_value;
}

uint64_t proc_cpu_usage(pid_t i_pid)
{
    return 0;
}

uint64_t proc_opened_fd_cnt(pid_t i_pid)
{
    uint64_t count = 0;
    char buff[20];
    snprintf(buff, sizeof(buff), "/proc/%d/fd", i_pid);
    DIR *dir = opendir(buff);
    if (dir) {
        while (readdir(dir) != NULL) {
            count++;
        }
        closedir(dir);
    }
    return count;
}

uint64_t proc_self_opened_fd_cnt()
{
    uint64_t count = 0;
    DIR *dir = opendir("/dev/fd"); // /proc/self/fd
    if (dir) {
        while (readdir(dir) != NULL) {
            count++;
        }
        closedir(dir);
    }
    return count;
}

uint64_t proc_rss_mem(pid_t i_pid)
{
    uint64_t r_value = false;
    
    do {
        auto proc_info = proc_status(i_pid);

        auto proc_rss = proc_info.find("VmRSS");
        if(proc_rss == proc_info.end()) { break; }

        r_value = strtol(proc_rss->second.c_str(), nullptr, 0);
        r_value = r_value * 1024; // 为了和macOS接口保持单位一致
    } while (false);
    
    return r_value;
}

uint64_t proc_virtual_mem(pid_t i_pid)
{
    return 0;
}


bool proc_flags_is_set(uint64_t i_flags, uint64_t i_flag_to_check)
{
    return (i_flags & i_flag_to_check) == i_flag_to_check;
}