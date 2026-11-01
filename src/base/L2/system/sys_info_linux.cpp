
#include "system/sys_info.hpp"

#include <cstring>
#include <fstream>
#include <sys/sysinfo.h>

#include "file/file_api.hpp"
#include "string/string_api.hpp"
#include "string/string_split.hpp"


time_t sys_start_time()
{
    static time_t r_start_time = 0;
    
    if(0 == r_start_time) {
        struct sysinfo sys_info = {};

        r_start_time = (0 == sysinfo(&sys_info)) ? (time(nullptr) - sys_info.uptime) : 0;
    }

    return r_start_time;
}

uint64_t sys_clock_tick()
{
    static time_t r_clock_tick = 0;

    if(0 == r_clock_tick) {
        r_clock_tick = sysconf(_SC_CLK_TCK);
    }

    return r_clock_tick;
}

uint64_t sys_time()
{
    struct timespec time{0,0}; clock_gettime(CLOCK_MONOTONIC_COARSE, &time);

    return (time.tv_sec * 1000 + time.tv_nsec / (1000 * 1000));
}

int64_t sys_cpu_num()
{
    return get_nprocs_conf();
}

std::map<std::string, std::string> sys_boot_config(int i_max_line)
{
    std::map<std::string, std::string> r_conf;

    do {
        // /proc/config.gz、/boot/config-xxx
        std::ifstream fs("/boot/config-" + sys_kern_release());
        if(!fs.is_open()) { break; }

        std::string line;
        for(int i = 0; i < i_max_line; i++) {
            if(!std::getline(fs, line)) { break; }

            if(!line.empty() && '#' != line.front()) {
                auto site = line.find('=');
                if(std::string::npos != site) {
                    std::string key = line.substr(0, site);
                    std::string val = line.substr(site + 1);
                    r_conf.insert({ key, val });
                }
            }
        }
    } while (false);

    return r_conf;
}

std::map<std::string, std::string> sys_os_release()
{
    std::map<std::string, std::string> ret_info;

    for(const auto &line : file_read_lines("/etc/os-release")) {
        auto kv_pair = string_pair(line, "=", true);

        kv_pair.first  = string_drop_quotes(kv_pair.first);
        kv_pair.second = string_drop_quotes(kv_pair.second);

        if(!kv_pair.first.empty()) { ret_info.insert( kv_pair ); }
    }

    return ret_info;
}

void sys_mounts(const std::string &i_path, const std::function<bool(const struct mntent *)> &i_callback)
{
    FileStreamPtr fs(setmntent(i_path.c_str(), "r"));
    if(nullptr == fs.get()) {
        return;
    }

    struct mntent *ent_ptr = nullptr;
    struct mntent  ent_tmp{};

    char buf[4 * FILENAME_MAX] = {};
    while( nullptr != (ent_ptr = getmntent_r(fs.get(), &ent_tmp, buf, sizeof(buf)))) {
        if(!i_callback(&ent_tmp)) {
            break;
        }
        bzero(&ent_tmp, sizeof(ent_tmp));
    }
}

std::string sys_mount_dir_for_cgroup_cpu()
{
    std::string r_path;

    sys_mounts("/proc/mounts", [&r_path](const struct mntent *i_mount) {
        if(0 == strcmp(i_mount->mnt_type, "cgroup") && nullptr != hasmntopt(i_mount, "cpu")) {
            r_path = i_mount->mnt_dir;
        }
        return r_path.empty();
    });

    return r_path;
}

bool sys_is_mount_cgroup_cpu()
{
    return sys_mount_dir_for_cgroup_cpu().empty();
}

bool sys_is_enable_cgroup_cpu()
{
    bool r_is_enable = false;

    file_read_lines("/proc/cgroups", [&r_is_enable](const std::string &i_line){
        const auto items = string_split<std::vector<std::string>>(i_line, "\t");

        if(items.size() >= 4 && items.front() == "cpu") {
            r_is_enable = (items[3] == "1");
        }
        return !r_is_enable;
    });

    return r_is_enable;
}
