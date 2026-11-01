
#include "system/sys_info.hpp"

#include <cstring>
#include <sys/utsname.h>
#include <unistd.h>

int64_t sys_page_size()
{
    return sysconf(_SC_PAGESIZE);
}

std::string sys_machine()
{
    struct utsname un; bzero(&un, sizeof(un));
    return uname(&un) == 0 ? un.machine : "";
}
std::string sys_hostname()
{
    struct utsname un; bzero(&un, sizeof(un));
    return uname(&un) == 0 ? un.nodename : "";
}
std::string sys_processor()
{
    return "";
}

std::string sys_kern_name()
{
    struct utsname un; bzero(&un, sizeof(un));
    return uname(&un) == 0 ? un.sysname : "";
}
std::string sys_kern_version()
{
    struct utsname un; bzero(&un, sizeof(un));
    return uname(&un) == 0 ? un.version : "";
}
std::string sys_kern_release()
{
    struct utsname un; bzero(&un, sizeof(un));
    return uname(&un) == 0 ? un.release : "";
}
