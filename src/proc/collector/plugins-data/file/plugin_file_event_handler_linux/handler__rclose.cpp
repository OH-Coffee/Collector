#include "matcher/matcher_factory.hpp"

#include "plugins-data/file/plugin_file_event_handler.hpp"

ret_code event_handler_for_close_read(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        static ReverseCharTreeMatcher matcher(get_file_rclose_event_inside_name());

        if(i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR; break; }

        // 仅需要特定后缀、特定文件名
        const auto file_path = i_msg->body.rclose.path;
        // if(RC_OK != (r_code = (matcher.match(file_path) ? RC_OK : RC_UNCONCERNED))) {
        //     break;
        // }
        
        const pid_t proc_pid = i_msg->head.event_process.pid;

        ProcInfo    proc_info;
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(proc_pid);
        if(nullptr != proc_info_ptr) {
            ;
        } else {
            if (try_get_proc_base_info(proc_pid, proc_info)) {
                ProcCache::ins().insert(proc_pid, proc_info_ptr = std::make_shared<ProcInfo>(proc_info));

                LOG_DIAGN("[PluginFile]\t[YES] 进程：%d 在缓存中未查到该数据 -> 再次获取成功; 错误码：%d", proc_pid, RC_OK);
            } else {
                LOG_DIAGN("[PluginFile]\t[ NO] 进程：%d 再次尝试获取仍然失败 -> 数据即将丢弃; 错误码：%d", proc_pid, RC_CACHE_ERROR);
                break;
            }
        }

        SCHEMA_HEAD *head = &o_msg->Head;
        SCHEMA_BODY *body = &o_msg->Body;
        std::memset(body, 0, sizeof(*body));
        set_header(i_msg->head.event_type, proc_info_ptr, head);

        EVENT_FILE_RCLOSE *file_info = &body->rclose;
        {
            string_copy2(file_path, sizeof(file_info->path), file_info->path);
            string_copy2(proc_info_ptr->path, sizeof(file_info->path_proc), file_info->path_proc);
            
            get_lstat(file_info->path, file_info->stat);
            if(!S_ISREG(file_info->stat.st_mode)) {
                r_code = RC_UNCONCERNED; break;
            }
        }

        ProcInfoPtr parent_proc = ProcCache::ins().find(proc_info_ptr->ppid);
        {
            if(nullptr != parent_proc) {
                string_copy2(parent_proc->path, sizeof(file_info->path_pproc), file_info->path_pproc);
            }
        }

        r_code = filter_match(global::filter(), o_msg, proc_info_ptr, parent_proc);
        if(RC_OK != r_code) {
            break;
        }

        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}
