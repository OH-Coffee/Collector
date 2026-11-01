
#include "plugins-data/proc/plugin_proc_event_handler.hpp"

ret_code event_handler_for_exec(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_msg == nullptr) { break; }
        if(o_msg == nullptr) { break; }

        pid_t       proc_pid  = i_msg->head.event_process.pid;
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(proc_pid);
        ProcInfo proc_info = {};
        if(nullptr != proc_info_ptr) {
            r_code = RC_OK;
        } else {
            if (try_get_proc_base_info(proc_pid, proc_info)) {
                proc_info.uid = i_msg->head.event_process.uid;
                proc_info.args = i_msg->body.exec.argv;
                proc_info_ptr = std::make_shared<ProcInfo>(proc_info);
                ProcCache::ins().insert(proc_pid, proc_info_ptr);
                LOG_DIAGN("[PluginProc]\t[ YES] 进程：%d 在缓存中未查到该数据 -> 再次获取成功; 错误码：%d", proc_pid, RC_OK);
            } else {
                r_code = RC_CACHE_ERROR;
                LOG_DIAGN("[PluginProc]\t[ NO] 进程：%d 再次尝试获取仍然失败 -> 再次获取仍然无法获取 -> 数据即将丢弃; 错误码：%d", proc_pid, r_code);
                break;
            }
        }
        if(proc_flags_is_set(proc_info_ptr->flags, PF_KTHREAD)) { break; }

        SCHEMA_HEAD *head = &o_msg->Head;
        SCHEMA_BODY *body = &o_msg->Body;
        std::memset(body, 0, sizeof(*body));
        set_header(i_msg->head.event_type, proc_info_ptr, head);

        EVENT_PROC_EXEC *body_proc = &body->exec;
        {
            body_proc->pid        = proc_info_ptr->pid;
            body_proc->ppid       = proc_info_ptr->ppid;
            body_proc->uid        = proc_info_ptr->uid;
            body_proc->gid        = proc_info_ptr->pgid;
            body_proc->sid        = proc_info_ptr->sid;
            body_proc->start_time = proc_info_ptr->stime;

            string_copy2(proc_info_ptr->name, sizeof(body_proc->name), body_proc->name);
            string_copy2(proc_info_ptr->path, sizeof(body_proc->path), body_proc->path);
            string_copy2(proc_info_ptr->args, sizeof(body_proc->args), body_proc->args);
            string_copy2(proc_info_ptr->work_dir, sizeof(body_proc->work_dir), body_proc->work_dir);
            string_copy2(proc_info_ptr->envs, sizeof(body_proc->env), body_proc->env);
            get_stat(proc_info_ptr->path.c_str(), body_proc->path_stat);
        }

        ProcInfoPtr parent_proc = ProcCache::ins().find(proc_info_ptr->ppid);
        if(nullptr != parent_proc) {
            string_copy2(parent_proc->path, sizeof(body_proc->ppath), body_proc->ppath);
        }

        r_code = filter_match(global::filter(), o_msg, proc_info_ptr, parent_proc);
        if(RC_OK != r_code) {
            break;
        }

        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}
