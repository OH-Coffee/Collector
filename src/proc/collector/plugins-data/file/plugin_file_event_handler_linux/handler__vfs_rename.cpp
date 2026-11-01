
#include <sys/sysmacros.h>

#include "system/sys_info.hpp"
#include "plugins-data/file/plugin_file_event_handler.hpp"

namespace {
    constexpr mode_t g_valid_types[] = { S_IFREG, S_IFLNK };

    typedef struct MountDirKey
    {
        uint64_t maj; uint64_t min; uint64_t idx;

        bool operator<(const MountDirKey &i_other) const {
            if (this->maj != i_other.maj) { return (this->maj < i_other.maj); }
            if (this->min != i_other.min) { return (this->min < i_other.min); }

            return (this->idx < i_other.idx);
        }
    } MountDirKey;
}

static std::map<MountDirKey, std::string> get_sys_mounts()
{
    std::map<MountDirKey, std::string> r_info;

    sys_mounts("/proc/mounts", [&r_info](const struct mntent *i_info) {
        if(struct stat st{}; 0 == stat(i_info->mnt_dir, &st)) {
            r_info[{ major(st.st_dev), minor(st.st_dev), st.st_ino }] = i_info->mnt_dir;
        } return true;
    });

    return r_info;
}

ret_code event_handler_for_vfs_rename(const EventPtr &i_msg,  SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR; break; }

        const pid_t proc_pid      = i_msg->head.event_process.pid;
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
            const static auto mounts = get_sys_mounts();

            const EventBodyVfsRename      *rename = &i_msg->body.vfs_rename;
            const decltype(rename->mount) *mount  = &i_msg->body.vfs_rename.mount;
            string_copy2(proc_info_ptr->path, sizeof(file_info->path_proc), file_info->path_proc);

            std::string old_name = path_reverse(rename->oldname);
            std::string new_name = path_reverse(rename->newname);
            if(old_name.empty() || new_name.empty()) {
                break;
            }

            const std::string *dir = nullptr;
            if(const auto it = mounts.find({ mount->dev_maj, mount->dev_min, mount->inode }); it != mounts.end()) {
                dir = &it->second;
            }

            if(nullptr != dir) {
                if("/" == *dir) {
                    old_name = *dir + old_name;
                    new_name = *dir + new_name;
                } else {
                    old_name = *dir + "/" + old_name;
                    new_name = *dir + "/" + new_name;
                }

                string_copy2(old_name, sizeof(file_info->path), file_info->path);
                string_copy2(new_name, sizeof(file_info->target_path), file_info->target_path);
            }

            get_lstat(file_info->target_path, file_info->stat);
            
            const auto mode = file_info->stat.st_mode;
            // if(!is_valid_type(mode, g_valid_types, std::size(g_valid_types))) {
            //     r_code = RC_UNCONCERNED; break;
            // }
        }

        ProcInfoPtr parent_proc = ProcCache::ins().find(proc_info_ptr->ppid);
        if(nullptr != parent_proc) {
            string_copy2(parent_proc->path, sizeof(file_info->path_pproc), file_info->path_pproc);
        }

        r_code = filter_match(global::filter(), o_msg, proc_info_ptr, parent_proc);
        if(RC_OK != r_code) {
            break;
        }

        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}
