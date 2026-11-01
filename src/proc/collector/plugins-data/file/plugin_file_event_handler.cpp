
#include "plugins-data/file/plugin_file_event_handler.hpp"


#if defined(OS_MACOS)
    typedef ret_code (*event_handler)(const es_message_t *, SCHEMA *);

    static const std::map<EventType, event_handler> g_event_handler_map = {
        { EVENT_TYPE_OPEN,              event_handler_for_open          },
        { EVENT_TYPE_CLOSE,             event_handler_for_close         },
        { EVENT_TYPE_CREATE,            event_handler_for_create        },
        { EVENT_TYPE_RENAME,            event_handler_for_rename        },
    };

    static ret_code event_handler_for_file(const EventPtr &, SCHEMA *);
#elif defined(OS_LINUX)
    typedef ret_code (*event_handler)(const EventPtr &i_msg, SCHEMA *o_msg);

    static std::map<EventType, event_handler> g_event_handler_map = {
        { EVENT_TYPE_RCLOSE,        event_handler_for_close_read    },
        { EVENT_TYPE_WCLOSE,      event_handler_for_close_write   },
        { EVENT_TYPE_RENAME,            event_handler_for_rename        },
        { EVENT_TYPE_RENAMEAT,          event_handler_for_rename        },
        { EVENT_TYPE_RENAMEAT2,         event_handler_for_rename        },
        { EVENT_TYPE_VFS_RENAME,        event_handler_for_vfs_rename    },
    };
#endif


ret_code PluginFile::event_handler(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { break; }
        
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

ret_code PluginFile::event_handler_run(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;
    do {
        static std::atomic_uint64_t lost_counter = 0;

        if(RC_OK == MessageClient::is_full())  {
            ++lost_counter; r_code = RC_THREAD_POLL_TASK_QUEUE_FULL; break;
        }

        SCHEMA msg; schema_init(&msg, sizeof(msg));

#if   defined(OS_MACOS)
        r_code = event_handler_for_file(i_msg, &msg);
#elif defined(OS_LINUX)
        auto event = g_event_handler_map.find(i_msg->head.event_type);
        if(event == g_event_handler_map.end()) {
            ++lost_counter; r_code = RC_UNCONCERNED; break;
        }

        r_code = event->second(i_msg, &msg);
#endif
        
        if(RC_OK == r_code) {
            // if(!IS_SET(this->_op_code, msg.Head.event_type)) { break; }
            
            r_code = MessageClient::send(DATA_MSG_SEND_TO_PROXY,
                                         reinterpret_cast<uint8_t *>(&msg),
                                         sizeof(msg));
            if(RC_OK != r_code && ++lost_counter % 100 == 0) {
                LOG_ERROR("[PluginFile]\t[ NO] 数据消息发送 -> 累计丢失数：%lld; 错误码：%d", lost_counter.load(), r_code);
            }
        } else if( !(RC_FILTER_MATCHED == r_code || RC_UNCONCERNED == r_code) ) {
            if(0 == (++lost_counter % 100)) {
                LOG_WARN("[PluginFile]\t[ NO] 事件采集完成 -> PID：%d，累计丢失数：%lld; 错误码：%d",
                         i_msg->head.event_process.pid,
                         lost_counter.load(), r_code);
            }
        }
    } while (false);

    return r_code;
}

#ifdef OS_MACOS
static ret_code event_handler_for_file(const EventPtr &i_event_msg, SCHEMA *o_cardinal_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_event_msg    == nullptr) { break; }
        if(o_cardinal_msg == nullptr) { break; }
        
        SCHEMA_HEAD *head = &o_cardinal_msg->Head;
        SCHEMA_BODY *body = &o_cardinal_msg->Body;
        {
            head->OperType      = OP_MIN;
            head->DataLen       = sizeof(SCHEMA_BODY);
            head->MessageType   = MESSAGE_DATA;
        }
        
        auto es_client    = i_event_msg->body.es->client;
        auto es_message   = i_event_msg->body.es->message;
        if(nil == es_client || nil == es_message) {
            break;
        }
        auto es_proc_info = es_message->process;
        
        auto event_hander = g_event_handler_map.find((EventType)es_message->event_type);
        if(g_event_handler_map.end() == event_hander) {
            r_code = RC_UNCONCERNED; break;
        }
        
        pid_t pid           = audit_token_to_pid(es_proc_info->audit_token);
        pid_t ppid          = es_proc_info->ppid;
        pid_t pid_version   = audit_token_to_pidversion(es_proc_info->audit_token);
        
        FILE_GEN_INFO *file_info = &body->FileAuditOpenEvent;
        {
            head->ProcessId     = pid;
            head->ParentId      = ppid;
            head->AuditToken    = es_proc_info->audit_token;
        }

        {
            guid_set(pid, es_proc_info->start_time, pid_version, head->ProcessGuid);
            
            ret_code r_fill = guid_set_with_cache(ppid, head->ParentGuid);
            if(RC_OK != r_fill && ppid > 0) {
                LOG_DEBUG("[PluginFile]\t[ NO] 文件事件-GUID获取：%d，错误码：%d", ppid, r_fill);
            }
        }
        
        ProcInfoPtr proc  = ProcCache::ins().find(pid);
        ProcInfoPtr pproc = ProcCache::ins().find(ppid);
        
        if(RC_OK != (r_code = event_hander->second(es_message, o_cardinal_msg))) {
            break;
        }
        
        if(nullptr != pproc) {
            guid_copy(pproc->guid, head->ParentGuid);
            string_copy2(pproc->path, C_MAX_PATH_SIZE, file_info->ParentPath);
        } else {
            guid_calculate(ppid, head->ParentGuid);
            string_copy2(proc_path(ppid), C_MAX_PATH_SIZE, file_info->ParentPath);
        }
        es_string_copy2(&es_proc_info->executable->path, C_MAX_PATH_SIZE, file_info->ProcessPath);
        
        if(RC_OK != (r_code = match__aegis_dlh(head, file_info->ProcessPath))) { break; }
        if(RC_OK != (r_code = match__aegis_mix(o_cardinal_msg, proc, pproc)))  { break; }

        EVENT_COUNTER(r_code, head);
    } while (false);
    
    return r_code;
}
#endif


const std::list<std::string> &get_file_rclose_event_inside_name()
{
    static const std::list<std::string> path = {
#if defined(OS_LINUX)
        "/.ssh/id_rsa",
        "/.ssh/known_hosts",
        "/.ssh/config",
        "/.bash_history",
        "/.zsh_history",
        "/.config/qaxbrowser/Default/Cookies",
        "/.config/qaxbrowser/Default/Safe Browsing Cookies",
        "/.config/qaxbrowser/Default/Login Data",
        "/.config/qaxbrowser/Default/Web Data",
        "/.config/qaxbrowser/Default/Safe Browsing Cookies",
        "/.config/browser360/Default/History",
        "/.config/browser360/Default/Cookies",
        "/.config/browser360/LoginData",
        "/.config/browser360/Web Data",
        "/.config/browser/Default/Web Data",
        "/.config/browser/Default/Visited Links",
        "/.config/browser/Default/Trust Tokens",
        "/.config/browser/Default/Top Sites",
        "/.config/browser/Default/Safe Browsing Cookies",
        "/.config/browser/Default/Affiliation Database",
#elif defined(OS_MACOS)
        "/360Bookmarks",
        "/360History",
        "/Archived History",
        "/Autofill",
        "/binarycookies",
        "/Bookmarks",
        "/com.apple.LaunchServices.QuarantineEventsV2",
        "/com.apple.VNCSettings.txt",
        "/Cookies",
        "/databases",
        "/History",
        "/kcpassword",
        "/krb5.keytab",
        "/leveldb",
        "/Login Data",
        "/sudoers",
        "/Password",
        "/Sync360_V8.sqlite3",
        "/SystemKey",
        "/SystemPolicy",
        "/Top Sites",
        "/Web Data",
        "/.bash_history",
        "/.zsh_history",
        "LastSession.plist",
        "Downloads.plist",
        "/id_rsa",
        "/known_hosts",
        "/Cookies.binarycookies",
        "/Cookies.plist",
        "/AutoFillCorrections.db",
        "/cookies.sqlite",
        "/formhistory.sqlite",
        "/logins.json",
        "/downloads.sqlite",
        "/webappsstore.sqlite",
        "/key4.db",
        "/places.sqlite",
        "/Autofills",
        "/NoteStore.sqlite",
        "/NoteStore.sqlite-wal",
        "/NoteStore.sqlite-shm",
        "/NotesV7.storedata",
        "/NotesV7.storedata-wal",
        "/NotesV7.storedata-shm",
        "/blockchain.dat",
        "/wallet.dat",
        "/app-store.json",
        ".wallet",
        "/userinfo.data",
        "sitemanager.xml",
        "recentservers.xml",
        "filezilla.xml",
#endif
    };
    
    return path;
}

const std::list<std::string> &get_file_wclose_event_inside_name()
{
    static const std::list<std::string> path = {
        ".plist",
        ".scptd",
        ".scpt",
        ".dylib",
        ".kext",
        ".applescript_compile",
        ".bzip2",
        ".dmg",
        ".gzip",
        ".jar",
        ".zip",
        ".tar",
        ".xar",
        ".rar",
        ".lua",
        ".py",
        ".sh",
        ".pl",
        ".jsp",
        ".jspx",
        ".php",
        ".asp",
        ".aspx",
        ".arj",
        ".cpio",
        ".cramfs",
        ".pkg",
        ".mpkg",
        ".lzh",
        ".lzma",
        ".lzma86",
        ".mslz",
        ".xar",
        ".xz",
        ".z",
        ".7z",
        ".cab",
        ".iso",
        ".gz",
        ".bz2",
        ".tgz",
        ".zshrc",
        ".zlogin",
        ".zwc",
        ".zshenv",
        ".zprofile",
        ".zlogout",
        ".bashrc",
        ".bash_profile",
        ".bash_prompt",
        ".applescript",
        ".db",
    };
    
    return path;
}

