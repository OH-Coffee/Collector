
#ifndef proc_info_macos_hpp
#define proc_info_macos_hpp

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <sys/types.h>

#include "process/proc_flag.hpp"

#if defined(OS_MACOS)
    #include <mach/message.h>
#include <bsm/libbsm.h>
#endif

// 当使用PROC_ARG_ALL时，proc_args_info()返回的数据按枚举值声明顺序排序
typedef enum : uint32_t
{
    PROC_ARG_UNKNOWN    =  (uint32_t)0x00,
    PROC_ARG_ARGC       =  (uint32_t)0x01 << 1,
    PROC_ARG_PATH       =  (uint32_t)0x01 << 2,
    PROC_ARG_ARGV       =  (uint32_t)0x01 << 3,
    PROC_ARG_ENVS       =  (uint32_t)0x01 << 4,
    PROC_ARG_ALL        = ~(uint32_t)0x00,
} ProcArgsInfo;

// 用于从proc_stat()的返回值中索引字段
typedef enum : uint32_t
{
    PROC_STAT_PID           = 0,
    PROC_STAT_NAME          = 1,
    PROC_STAT_STATE         = 2,
    PROC_STAT_PPID          = 3,
    PROC_STAT_PGID          = 4,
    PROC_STAT_SID           = 5,
    PROC_STAT_TTY_NR        = 6,
    PROC_STAT_TTY_PGID      = 7,
    PROC_STAT_FLAGS         = 8,
    PROC_STAT_USER_TIME     = 13,
    PROC_STAT_SYS_TIME      = 14,
    PROC_STAT_PRIORITY      = 17,
    PROC_STAT_NICE          = 18,
    PROC_STAT_THREAD_NUM    = 19,
    PROC_STAT_START_TIME    = 21,
    PROC_STAT_START_STACK   = 27,
    PROC_STAT_START_BREAK   = 46,
    PROC_STAT_UNKNOWN       = UINT32_MAX,
} ProcStatInfo;

char *      proc_readlink(const std::string &i_path, uint32_t i_buf_len, char *o_buf);
std::string proc_readlink(const std::string &i_path);
std::string proc_read_all(const std::string &i_path, char i_replacement = ' ', size_t i_max = 1024 * 4);

int         proc_all_pids(pid_t o_pid_list[], int i_list_size);
int         proc_pids_by_name(const char *i_name, std::vector<pid_t> &o_pids, bool i_ignore_case = false);
int         proc_pids_by_path(const char *i_path, std::vector<pid_t> &o_pids, bool i_ignore_case = false);

char *      proc_name(pid_t i_pid, uint32_t i_buf_len, char *o_buf);
std::string proc_name(pid_t i_pid);

char *      proc_path(pid_t i_pid, uint32_t i_buf_len, char *o_buf);
std::string proc_path(pid_t i_pid);
char *      proc_work_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf);
std::string proc_work_dir(pid_t i_pid);
char *      proc_root_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf);
std::string proc_root_dir(pid_t i_pid);

std::vector<std::string>           proc_stat(pid_t i_pid);
std::map<std::string, std::string> proc_status(pid_t i_pid);

std::vector<std::string> proc_args_info(pid_t i_pid, ProcArgsInfo i_type);

std::string proc_args(pid_t i_pid);
std::string proc_envs(pid_t i_pid);
std::string proc_cmdline(pid_t i_pid);

bool        proc_threads(pid_t i_pid, std::vector<uint64_t> &o_tid);
bool        proc_threads_cpu(pid_t i_pid, std::map<uint64_t, int> &o_cpu_info);
uint64_t    proc_cpu_time(pid_t i_pid);      // 单位：ms
uint64_t    proc_cpu_usage(pid_t i_pid);     // 单位：百分比
uint64_t    proc_opened_fd_cnt(pid_t i_pid); // 进程打开的文件描述符数量
uint64_t    proc_self_opened_fd_cnt(); // 当前进程打开的文件描述符数量

uint64_t    proc_rss_mem(pid_t i_pid);      // 单位：字节
uint64_t    proc_virtual_mem(pid_t i_pid);  // 单位：字节

#if defined(OS_MACOS)
bool        proc_audit_token(pid_t i_pid, audit_token_t &o_token);
#endif
bool        proc_flags_is_set(uint64_t i_flags, uint64_t i_flag_to_check);

#endif /* proc_info_macos_hpp */
