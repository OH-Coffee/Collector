
#include <sys/stat.h>

#include "cache/proc_cache_define.hpp"
#include "file/file_type.hpp"
#include "string/string_api.hpp"
#include "system/sys_info.hpp"

#include "plugin/plugin_helper.hpp"

#include "plugins-data/cache/plugin_cache.hpp"

ret_code PluginCache::event_handler(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { r_code = RC_UNCONCERNED; break; }
        
        if(_threads.get_thread_size() <= 0) {
            r_code = this->event_handler_run(i_msg);
        } else {
            r_code = _threads.add_task([this, i_msg](){
                this->event_handler_run(i_msg);
            });
        }
    } while (false);

    return r_code;
}

ret_code PluginCache::event_handler_run(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_msg->head.event_type) {
        case EVENT_TYPE_CLONE:      { r_code = event_handler_for_fork(i_msg);   } break;
        case EVENT_TYPE_EXEC_ENTRY: 
        case EVENT_TYPE_EXEC:       { r_code = event_handler_for_exec(i_msg);   } break;
        case EVENT_TYPE_EXIT_GROUP: { r_code = event_handler_for_exit(i_msg);   } break;
        default:                    { r_code = RC_UNCONCERNED;                  } break;
    }

    return r_code;
}

ret_code PluginCache::event_handler_for_exit(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_msg) { r_code = RC_ARG_ERR; break; }

        r_code = _pid_cache.insert(i_msg->head.event_process.pid);
        if(RC_OK != r_code) {
            r_code = ProcCache::ins().erase(i_msg->head.event_process.pid);
        }
    } while (false);


    return r_code;
}

static ProcInfoPtr get_and_update_proc_cache(const EventPtr &i_msg)
{
    ProcInfoPtr r_info = nullptr;

    do {
        if(nullptr == i_msg) { break; }

        ret_code r_code = RC_UNKNOWN;
        if(nullptr != (r_info = NEW_PROC_INFO())) {
            static constexpr std::hash<std::string> hash;

            r_info->pid = i_msg->head.event_process.pid;
            if(EVENT_TYPE_CLONE == i_msg->head.event_type) {
                r_info->pid = i_msg->body.clone.pid_parent;
            }

            r_info->path     = proc_path(r_info->pid);
            r_info->work_dir = proc_work_dir(r_info->pid);
            // r_info->root_dir = proc_root_dir(r_info->pid);

            const auto process_stat = proc_stat(r_info->pid);
            if(process_stat.empty()) {
                r_code = RC_PROC_GET_STAT_ERR; break;
            }

            if(EVENT_TYPE_EXEC_ENTRY == i_msg->head.event_type && '\0' != i_msg->body.exec.argv[0]) {
                r_info->args = i_msg->body.exec.argv;
            } else {
                r_info->args = proc_args(r_info->pid);
            }
//            r_info->envs = proc_envs(r_info->pid);

            r_info->ppid    = string_to_integer<pid_t>(process_stat[PROC_STAT_PPID]);
            r_info->pgid    = string_to_integer<pid_t>(process_stat[PROC_STAT_PGID]);
            r_info->uid     = i_msg->head.event_process.uid;
            r_info->sid     = string_to_integer<pid_t>(process_stat[PROC_STAT_SID]);
            r_info->flags   = string_to_integer<uint64_t>(process_stat[PROC_STAT_FLAGS]);

            const auto proc_start_jiffies = string_to_integer<time_t>(process_stat[PROC_STAT_START_TIME]);
            r_info->stime.tv_sec  = sys_start_time() + (proc_start_jiffies / sys_clock_tick());
            r_info->stime.tv_usec = 0;

            const uint32_t pid_version = hash(r_info->path + r_info->args);

            const std::string process_path = proc_path(r_info->pid);
            if(!process_path.empty()) {
                r_info->path = process_path;
            }

            if(EVENT_TYPE_EXEC_ENTRY == i_msg->head.event_type && '\0' != i_msg->body.exec.path[0]) {
                if('/'  == i_msg->body.exec.path[0]) {
                    r_info->path = i_msg->body.exec.path;
                } else if(!r_info->work_dir.empty()) {
                    const auto path = file_realpath(r_info->work_dir + "/" + i_msg->body.exec.path);
                    const auto type = file_type(path, FT_CLASS_BINARY).binary_t;
                    if(!path.empty() && (FT_BINARY_ELF_EXEC == type || FT_BINARY_ELF_DYN == type)) {
                        r_info->path = path;
                    }
                }
            }

            struct stat file_stat = {};
            if(0 != lstat(r_info->path.c_str(), &file_stat) || S_ISLNK(file_stat.st_mode)) {
                r_info->path = proc_path(r_info->pid);
            }
            
            r_info->name = proc_name(r_info->pid);

            if(r_info->name.empty()) { r_info->name = process_stat[PROC_STAT_NAME]; }
            if(r_info->path.empty()) { r_info->path = r_info->name;                 }
            if(r_info->args.empty()) { r_info->args = r_info->name;                 }

            r_code = ProcCache::ins().insert(ProcCacheKey(r_info->pid, pid_version), r_info);
        }
        if(RC_OK == r_code) {
            LOG_DIAGN("[PluginCache]\t[YES] 进程：%d 缓存添加成功，事件类型：%d；错误码：%d", r_info->pid, i_msg->head.event_type, r_code);
        } else {
            LOG_DIAGN("[PluginCache]\t[ NO] 进程：%d 缓存添加成功，事件类型：%d；错误码：%d", r_info->pid, i_msg->head.event_type, r_code);
        }
    } while (false);

    return r_info;
}

static inline ret_code event_handler_for_exec_with_bpf(const EventPtr &i_msg)
{
    return (nullptr != get_and_update_proc_cache(i_msg)) ? RC_OK : RC_CACHE_ERROR;
}

static inline ret_code event_handler_for_exec_with_connector(const EventPtr &i_msg)
{
    return (nullptr != get_and_update_proc_cache(i_msg)) ? RC_OK : RC_CACHE_ERROR;
}

ret_code PluginCache::event_handler_for_fork(const EventPtr &i_msg)
{
    return (nullptr != get_and_update_proc_cache(i_msg)) ? RC_OK : RC_CACHE_ERROR;
}

ret_code PluginCache::event_handler_for_exec(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_msg->head.event_producer) {
        case EP_BPF_TRACEPOINT:    { r_code = event_handler_for_exec_with_bpf(i_msg);       } break;
        case EP_NETLINK_CONNECTOR: { r_code = event_handler_for_exec_with_connector(i_msg); } break;
        default: { } break;
    }

    return r_code;
}

ret_code PluginCache::when_pid_insert(pid_t)
{
    return RC_OK;
}

ret_code PluginCache::when_pid_delete(pid_t i_pid)
{
    return ProcCache::ins().erase(i_pid);
}
