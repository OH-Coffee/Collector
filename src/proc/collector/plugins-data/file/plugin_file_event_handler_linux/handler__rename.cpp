#include "plugins-data/file/plugin_file_event_handler.hpp"

namespace {
    constexpr mode_t g_valid_types[] = { S_IFREG, S_IFLNK };
}

ret_code event_handler_for_rename(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR; break; }

        pid_t       proc_pid  = i_msg->head.event_process.pid;
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(proc_pid);
        if(nullptr != proc_info_ptr) {
            ;
        } else {
            if (ProcInfo proc_info; try_get_proc_base_info(proc_pid, proc_info)) {
                proc_info_ptr = std::make_shared<ProcInfo>(proc_info);
                ProcCache::ins().insert(proc_pid, proc_info_ptr);
                LOG_DIAGN("[PluginFile]\t[ YES] 进程：%d 在缓存中未查到该数据 -> 再次获取成功; 错误码：%d", proc_pid, RC_OK);
            } else {
                LOG_DIAGN("[PluginFile]\t[ NO] 进程：%d 再次尝试获取仍然失败 -> 数据即将丢弃; 错误码：%d", proc_pid, RC_CACHE_ERROR);
                break;
            }
        }

        SCHEMA_HEAD *head = &o_msg->Head;
        SCHEMA_BODY *body = &o_msg->Body;

        std::memset(body, 0, sizeof(*body));
        set_header(i_msg->head.event_type, proc_info_ptr, head);

        EVENT_FILE_RENAME *file_info = &body->rename;
        {
            const std::string      format = "/proc/%d/fd/%ld";
            const EventBodyRename *rename = &i_msg->body.rename;
            string_copy2(proc_info_ptr->path, sizeof(file_info->path_proc), file_info->path_proc);

            if('\0' == rename->oldname[0] || '\0' == rename->newname[0]) {
                break;
            }

            std::string work_dir = proc_info_ptr->work_dir;
            if(work_dir.size() > 1 && work_dir.back() != '/') {
                work_dir.push_back('/'); // 添加路径分隔符，后续无需添加
            }

            std::string old_path(1024, 0); old_path = work_dir;
            std::string new_path(1024, 0); new_path = work_dir;

            if(rename->olddfd >= 0 && !IS_SET(rename->olddfd, (unsigned int)AT_FDCWD)) {
                snprintf(old_path.data(), old_path.size() - 1, format.c_str(), proc_pid, rename->olddfd);
                old_path = proc_readlink(old_path);
            }

            if(rename->newdfd >= 0 && !IS_SET(rename->newdfd, (unsigned int)AT_FDCWD)) {
                snprintf(new_path.data(), new_path.size() - 1, format.c_str(), proc_pid, rename->newdfd);
                new_path = proc_readlink(new_path);
            }

            if(old_path.size() > 1 && old_path.back() != '/') { old_path.push_back('/'); }
            if(new_path.size() > 1 && new_path.back() != '/') { new_path.push_back('/'); }

            if('/' == rename->oldname[0]) {
                string_copy2(rename->oldname, sizeof(file_info->path), file_info->path);
            } else {
                string_copy2(old_path + rename->oldname, sizeof(file_info->path), file_info->path);
            }

            if('/' == rename->newname[0]) {
                string_copy2(rename->newname, sizeof(file_info->target_path), file_info->target_path);
            } else {
                string_copy2(new_path + rename->newname, sizeof(file_info->target_path), file_info->target_path);
            }

            get_lstat(file_info->target_path, file_info->stat);
            
            const auto mode = file_info->stat.st_mode;
            if(!is_valid_type(mode, g_valid_types, std::size(g_valid_types))) {
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
