
#include <netinet/in.h>
#include <cstring>
#include <ctime>

#include "file/file_api.hpp"
#include "system/sys_info.hpp"

#include "plugin/plugin_helper.hpp"



ret_code filter_match(filter::Filter &i_filter,
                      SCHEMA *i_msg,
                      const ProcInfoPtr &i_proc,
                      const ProcInfoPtr &i_pproc)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_msg) { r_code = RC_OK; break; }

        filter::StructSet structs;

        if(nullptr != i_msg)   { structs.msg   = i_msg;   }
        if(nullptr != i_proc)  { structs.proc  = i_proc;  }
        if(nullptr != i_pproc) { structs.pproc = i_pproc; }

        r_code = i_filter.match(i_msg->Head.event_type, structs);

        if(RC_FILTER_MATCH_EXCLUEDE == r_code) { r_code = RC_UNCONCERNED; break; }
        if(RC_FILTER_MATCH_INCLUEDE == r_code) { r_code = RC_OK; break;          }
        if(RC_FILTER_MATCH_UNKNOWN  == r_code) { break; }
    } while (false);

    return r_code;
}

void copy_stat(struct stat &i_stat, FileStat &o_stat)
{
    o_stat.gid = i_stat.st_gid;
    o_stat.uid = i_stat.st_uid;

#if defined(OS_MACOS)
    o_stat.atime = i_stat.st_atimespec;
    o_stat.mtime = i_stat.st_mtimespec;
    o_stat.ctime = i_stat.st_ctimespec;
#else
    o_stat.atime = i_stat.st_atim;
    o_stat.mtime = i_stat.st_mtim;
    o_stat.ctime = i_stat.st_ctim;
#endif

    o_stat.st_size  = i_stat.st_size;
    o_stat.st_mode  = i_stat.st_mode;
    o_stat.st_nlink = i_stat.st_nlink;
}

void get_stat(const char *i_path, FileStat &o_stat)
{
    struct stat file_stat {};
    if(0 == stat(i_path, &file_stat)) {
        copy_stat(file_stat, o_stat);
    }
}
void get_stat(const uint8_t *i_path, FileStat &o_stat)
{ 
    get_stat((char *)i_path, o_stat);
}

void get_lstat(const char *i_path, FileStat &o_stat)
{
    struct stat file_stat {};
    if(0 == lstat(i_path, &file_stat)) {
        copy_stat(file_stat, o_stat);
    }
}
void get_lstat(const uint8_t *i_path, FileStat &o_stat)
{
    get_lstat((char *)i_path, o_stat);
}

void set_header(EventType i_event, const ProcInfoPtr &i_proc_info, SCHEMA_HEAD *o_header)
{
    std::memset(o_header, 0, sizeof(*o_header));
    std::memcpy(o_header->magic, SCHEMA_HEAD_MAGIC, sizeof(SCHEMA_HEAD_MAGIC));

    o_header->version     = SCHEMA_HEAD_VERSION;
    o_header->type        = SCHEMA_EVENT;
    o_header->pid         = i_proc_info->pid;
    o_header->ppid        = i_proc_info->ppid;
    o_header->time        = static_cast<uint64_t>(std::time(nullptr));
    o_header->event_type  = i_event;
    o_header->event_class = EVENT_CLASS_UNKNOWN;
    o_header->data_len    = sizeof(SCHEMA_BODY);
}

ret_code event_register(const char              *i_name,
                        EventRegisterArgs        i_args,
                        message::MessageClient  *i_client)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        auto msg_ptr = NEW_MESSAGE(CTRL_MSG_DRIVER_EVENT_REGISTER, nullptr, sizeof(EventRegisterArgs));

        auto args_ptr = (EventRegisterArgs *)msg_ptr->buf;
        args_ptr->event      = i_args.event;
        args_ptr->observer   = i_args.observer;
        
        if(RC_OK == (r_code = i_client->send_syn(msg_ptr))) {
            LOG_INFO( "[%s]\t[YES] 事件：%d 注册成功；错误码：%d", i_name, args_ptr->event, r_code);
        } else {
            LOG_ERROR("[%s]\t[ NO] 事件：%d 注册成功；错误码：%d", i_name, args_ptr->event, r_code);
        }
    } while (false);

    return r_code;
}

ret_code event_unregister_all(const char              *i_name,
                              EventRegisterArgs        i_args,
                              message::MessageClient  *i_client)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        auto msg_ptr = NEW_MESSAGE(CTRL_MSG_DRIVER_EVENT_UNREGISTER_ALL, nullptr, sizeof(EventRegisterArgs));
        
        auto args_ptr = (EventRegisterArgs *)msg_ptr->buf;
        args_ptr->event      = EVENT_TYPE_ALL;
        args_ptr->observer   = i_args.observer;
        
        if(RC_OK == (r_code = i_client->send_syn(msg_ptr))) {
            LOG_INFO( "[%s]\t[YES] 全事件注销成功；错误码：%d", i_name, r_code);
        } else {
            LOG_ERROR("[%s]\t[ NO] 全事件注销成功；错误码：%d", i_name, r_code);
        }
    } while (false);

    return r_code;
}

bool try_get_proc_base_info(const pid_t &i_pid, ProcInfo &o_proc_info)
{
    bool r_res = true;

    do {
        o_proc_info.pid = i_pid;

        const auto process_path = proc_path(i_pid);
        if (process_path.empty()) {
            r_res = false; break;
        }

        const auto process_stat = proc_stat(i_pid);
        {
            o_proc_info.path    = process_path;
            o_proc_info.name    = process_stat[PROC_STAT_NAME];
            o_proc_info.ppid    = string_to_integer<pid_t>(process_stat[PROC_STAT_PPID]);
            o_proc_info.pgid    = string_to_integer<pid_t>(process_stat[PROC_STAT_PGID]);
            o_proc_info.sid     = string_to_integer<pid_t>(process_stat[PROC_STAT_SID]);
            o_proc_info.flags   = string_to_integer<uint64_t>(process_stat[PROC_STAT_FLAGS]);
        }

        const auto proc_start_jiffies = string_to_integer<time_t>(process_stat[PROC_STAT_START_TIME]);
        {
            o_proc_info.stime.tv_sec  = sys_start_time() + (proc_start_jiffies / sys_clock_tick());
            o_proc_info.stime.tv_usec = 0;
        }
    } while (false);

    return r_res;
}
