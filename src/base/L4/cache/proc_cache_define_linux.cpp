
#include <algorithm>

#include "file/file_api.hpp"
#include "system/sys_info.hpp"
#include "process/proc_info.hpp"
#include "string/string_api.hpp"
#include "string/string_split.hpp"
#include "cache/proc_cache_define.hpp"

ret_code ProcCache::init()
{
    ret_code r_code = RC_UNKNOWN;

    this->disable(); // 缓存失能，避免内部、外部调用插入、查找;
    do {
        pid_t pids[3000]; bzero(pids, sizeof(pids));

        // 获取系统所有进程的PID，最大3k个
        int r_pid_num = proc_all_pids(pids, sizeof(pids));
        if(r_pid_num <= 0) { r_code = RC_PROC; break; }

        // 将PID按升序排序
        std::sort(pids, pids + r_pid_num);

        std::lock_guard<std::mutex> mutex_guard(_mutex);

        for(int i = 0; i < r_pid_num; ++i) {
            pid_t pid = pids[i]; if(pid <= 0) { continue; }

            // 获取进程BSD信息
            auto process_stat = proc_stat(pid);
            if(process_stat.empty()) {
                continue;
            }

            auto process_status = proc_status(pid);
            auto process_path   = proc_path(pid);
            auto process_args   = proc_args(pid);

            if(process_path.empty()) {
                process_path = process_stat[PROC_STAT_NAME];
            }

            // 创建缓存项
            auto result = _cache.emplace(pid, NEW_PROC_CACHE_ITEM());
            if(!result.second) { continue; }

            static constexpr std::hash<std::string> hash;
            uint32_t pid_version = hash(process_path + process_args);

            ProcCacheKey     key(pid, pid_version);
            ProcCacheItemPtr cache_item = result.first->second;

            // 缓存项初始化
            cache_item->last_time = 0;
            cache_item->last_info = std::make_shared<ProcInfo>();
            cache_item->proc_infos.emplace(key.version, cache_item->last_info);

            ProcInfoPtr proc = cache_item->last_info;

            // 进程信息初始化
            {
                proc->pid   = string_to_integer<pid_t>(process_stat[PROC_STAT_PID]);
                proc->ppid  = string_to_integer<pid_t>(process_stat[PROC_STAT_PPID]);

                proc->pgid  = string_to_integer<pid_t>(process_stat[PROC_STAT_PGID]);
                {
                    auto uid_str = process_status["Uid"];
                    auto uid_vec = string_split<std::vector<std::string>>(uid_str, "\t", true);
                    proc->uid = (uid_vec.size() == 4) ? string_to_integer<uid_t>(uid_vec[1]) : -1;
                }
                proc->sid   = string_to_integer<pid_t>(process_stat[PROC_STAT_SID]);
                proc->flags = string_to_integer<uint64_t>(process_stat[PROC_STAT_FLAGS]);

                auto proc_start_jiffies = string_to_integer<time_t>(process_stat[PROC_STAT_START_TIME]);
                proc->stime.tv_sec  = sys_start_time() + (proc_start_jiffies / sys_clock_tick());
                proc->stime.tv_usec = 0;
            }
            {
                proc->name = proc_name(proc->pid);
                proc->path = process_path;
                proc->args = process_args;
//                proc->envs = proc_envs(proc->pid);

                proc->work_dir = proc_work_dir(proc->pid);
//                proc->root_dir = proc_root_dir(proc->pid);

                if(proc->name.empty()) {
                    proc->name = file_name(proc->path);
                }
            }

            {

                ProcInfoPtr parent_proc = nullptr;

                auto proc_cache_item = _cache.find(proc->ppid);
                if(_cache.end() != proc_cache_item) {
                    parent_proc = proc_cache_item->second->last_info;
                }
            }
        }

        r_code = RC_OK;
    } while (false);
    this->enable();  // 缓存使能

    return r_code;
}