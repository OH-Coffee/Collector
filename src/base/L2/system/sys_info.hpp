
#ifndef sys_info_posix_hpp
#define sys_info_posix_hpp

#include <cstdint>
#include <ctime>
#include <map>
#include <string>
#include <sys/types.h>

#if defined(OS_LINUX)
    #include "system/sys_info_linux.hpp"
#endif

typedef enum : int64_t
{
    SYS_TYPE_UNKNOWN = 0,

    SYS_TYPE_CENTOS,
    SYS_TYPE_MACOS,
    SYS_TYPE_KYLIN,
    SYS_TYPE_REDHAT,
    SYS_TYPE_UBUNTU,
    SYS_TYPE_UOS,
} SysType;

SysType     sys_type();

time_t      sys_start_time();
uint64_t    sys_clock_tick();   // CPU每毫秒的tick值
uint64_t    sys_time();         // 系统时间，单位：毫秒

int64_t     sys_cpu_num();
int64_t     sys_page_size();

std::string sys_machine();
std::string sys_hostname();
std::string sys_processor();

std::string sys_kern_name();
std::string sys_kern_version();
std::string sys_kern_release();

std::map<std::string, std::string> sys_boot_config(int i_max_line = 10000);
std::map<std::string, std::string> sys_os_release();

#endif // sys_info_posix_hpp
