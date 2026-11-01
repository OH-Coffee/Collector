
#import <mach/mach_time.h>
#import <Foundation/Foundation.h>

#import "system/sys_info.hpp"

#include <sys/sysctl.h>

time_t sys_start_time()
{
    static time_t r_start_time = 0;
    
    if(0 == r_start_time) {
        timeval time = { 0, 0 };
        size_t  size = sizeof(time);
        
        r_start_time = (0 == sysctlbyname("kern.boottime", &time, &size, nullptr, 0)) ? time.tv_sec : 0;
    }

    return r_start_time;
}

uint64_t sys_clock_tick()
{
    mach_timebase_info_data_t timebase{};
    
    mach_timebase_info(&timebase);

    return (timebase.numer / timebase.denom) * 1000;
}

uint64_t sys_time()
{
    static const uint64_t tick = sys_clock_tick();
    
    return (mach_absolute_time() / tick);
}

int64_t sys_cpu_num()
{
    size_t ncpu = 0;
    size_t size = sizeof(ncpu);
    
    sysctlbyname("hw.logicalcpu_max", &ncpu, &size, nullptr, 0);
    
    return ncpu;
}

std::map<std::string, std::string> sys_boot_config(int i_max_line)
{
    return {};
}

std::map<std::string, std::string> sys_os_release()
{
    std::map<std::string, std::string> ret_info;

    do {
        NSData *data = [NSData dataWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
        if(nil == data) { break; }
        
        CFPropertyListRef plist = CFPropertyListCreateWithData(kCFAllocatorDefault,
                                                               (__bridge CFDataRef)data,
                                                               kCFPropertyListImmutable,
                                                               NULL, NULL);
        if(nil == plist) { break; }
        
        NSDictionary *dictionary = (__bridge NSDictionary *)plist;
        for(NSString *key in dictionary) {
            NSString *val = [dictionary objectForKey:key];
            ret_info.insert( { key.UTF8String, val.UTF8String} );
        }
    } while (false);

    return ret_info;
}
