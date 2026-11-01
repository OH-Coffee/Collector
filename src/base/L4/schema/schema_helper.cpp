#include <cstring>
#include <ctime>

#include "jsoncpp/json/json.h"
#include "schema/schema_helper.hpp"

namespace {

const std::map<uint64_t, std::string> g_event_type2name = {
    { EVENT_TYPE_FORK,              "EVENT_TYPE_FORK"            },
    { EVENT_TYPE_CLONE,             "EVENT_TYPE_CLONE"           },
    { EVENT_TYPE_EXEC,              "EVENT_TYPE_EXEC"            },
    { EVENT_TYPE_EXIT_GROUP,        "EVENT_TYPE_EXIT_GROUP"      },
    { EVENT_TYPE_EXEC_ENTRY,        "EVENT_TYPE_EXEC_ENTRY"      },
    { EVENT_TYPE_OPEN,              "EVENT_TYPE_OPEN"            },
    { EVENT_TYPE_RCLOSE,            "EVENT_TYPE_RCLOSE"          },
    { EVENT_TYPE_WCLOSE,            "EVENT_TYPE_WCLOSE"          },
    { EVENT_TYPE_RENAME,            "EVENT_TYPE_RENAME"          },
    { EVENT_TYPE_RENAMEAT,          "EVENT_TYPE_RENAMEAT"        },
    { EVENT_TYPE_RENAMEAT2,         "EVENT_TYPE_RENAMEAT2"       },
    { EVENT_TYPE_VFS_RENAME,        "EVENT_TYPE_VFS_RENAME"      },
    { EVENT_TYPE_CONNECT,           "EVENT_TYPE_CONNECT"         },
    { EVENT_TYPE_RECVFROM,          "EVENT_TYPE_RECVFROM"        },
    { EVENT_TYPE_RECVMSG,           "EVENT_TYPE_RECVMSG"         },
    { EVENT_TYPE_SKB_CONSUME_UDP,   "EVENT_TYPE_SKB_CONSUME_UDP" },
};

template <typename T, size_t N>
std::string buffer_to_string(const T (&i_buffer)[N])
{
    const void* terminator = std::memchr(i_buffer, '\0', N);
    const size_t length = (nullptr != terminator)
                        ? static_cast<const char*>(terminator) - reinterpret_cast<const char*>(i_buffer)
                        : N;

    return std::string(reinterpret_cast<const char*>(i_buffer), length);
}

void add_file_stat(Json::Value  &io_json,
                   const FileStat &i_stat,
                   const std::string &i_prefix)
{
    io_json[i_prefix + "uid"]         = i_stat.uid;
    io_json[i_prefix + "gid"]         = i_stat.gid;
    io_json[i_prefix + "atime_sec"]   = (Json::Int64)i_stat.atime.tv_sec;
    io_json[i_prefix + "atime_nsec"]  = (Json::Int64)i_stat.atime.tv_nsec;
    io_json[i_prefix + "mtime_sec"]   = (Json::Int64)i_stat.mtime.tv_sec;
    io_json[i_prefix + "mtime_nsec"]  = (Json::Int64)i_stat.mtime.tv_nsec;
    io_json[i_prefix + "ctime_sec"]   = (Json::Int64)i_stat.ctime.tv_sec;
    io_json[i_prefix + "ctime_nsec"]  = (Json::Int64)i_stat.ctime.tv_nsec;
    io_json[i_prefix + "mode"]        = (Json::UInt64)i_stat.st_mode;
    io_json[i_prefix + "nlink"]       = (Json::UInt64)i_stat.st_nlink;
}

void add_process_body(Json::Value &io_body, const EVENT_PROC_EXEC &i_data)
{
    io_body["pid"]             = i_data.pid;
    io_body["ppid"]            = i_data.ppid;
    io_body["uid"]             = i_data.uid;
    io_body["gid"]             = i_data.gid;
    io_body["sid"]             = i_data.sid;
    io_body["start_time_sec"]  = (Json::Int64)i_data.start_time.tv_sec;
    io_body["start_time_usec"] = (Json::Int64)i_data.start_time.tv_usec;
    io_body["name"]            = buffer_to_string(i_data.name);
    io_body["path"]            = buffer_to_string(i_data.path);
    io_body["args"]            = buffer_to_string(i_data.args);
    io_body["work_dir"]        = buffer_to_string(i_data.work_dir);
    io_body["tty"]             = buffer_to_string(i_data.tty);
    io_body["env"]             = buffer_to_string(i_data.env);
    io_body["ppath"]           = buffer_to_string(i_data.ppath);

    add_file_stat(io_body, i_data.path_stat, "path_");
}

void add_file_body(Json::Value &io_body, const EVENT_FILE_OPEN &i_data)
{
    io_body["flags"]        = i_data.flags;
    io_body["path"]         = buffer_to_string(i_data.path);
    io_body["target_path"]  = buffer_to_string(i_data.target_path);
    io_body["process_path"] = buffer_to_string(i_data.path_proc);
    io_body["parent_path"]  = buffer_to_string(i_data.path_pproc);

    add_file_stat(io_body, i_data.stat, "");
}

} // namespace

const std::map<uint64_t, std::string> &schema_events()
{
    return g_event_type2name;
}

ret_code schema_init(SCHEMA *i_schema, uint64_t i_schema_size)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_schema || i_schema_size < sizeof(SCHEMA)) {
            r_code = RC_ARG_ERR; break;
        }

        std::memset(i_schema, 0, sizeof(SCHEMA));
        std::memcpy(i_schema->Head.magic, SCHEMA_HEAD_MAGIC, sizeof(SCHEMA_HEAD_MAGIC));
        
        i_schema->Head.version     = SCHEMA_HEAD_VERSION;
        i_schema->Head.sequence    = 0;
        i_schema->Head.type        = SCHEMA_UNKNOWN;
        i_schema->Head.pid         = 0;
        i_schema->Head.ppid        = 0;
        i_schema->Head.time        = (uint64_t)std::time(nullptr);
        i_schema->Head.event_type  = EVENT_TYPE_UNKNOWN;
        i_schema->Head.event_class = EVENT_CLASS_UNKNOWN;
        i_schema->Head.data_len    = 0;
        i_schema->Head.reverse32   = 0;
        i_schema->Head.reverse64   = 0;

        r_code = RC_OK;
    } while(false);

    return r_code;
}

ret_code schema_check(const SCHEMA *i_schema)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_schema) { r_code = RC_ARG_ERR; break; }

        bool magic_cmp = std::memcmp(i_schema->Head.magic,
                                     SCHEMA_HEAD_MAGIC,
                                     sizeof(SCHEMA_HEAD_MAGIC));

        if(0 == magic_cmp && i_schema->Head.version == SCHEMA_HEAD_VERSION) {
            r_code = RC_OK;
        } else {
            r_code = RC_FAIL;
        }
    } while(false);

    return r_code;
}

std::string schema2string(const SCHEMA &i_schema)
{
    return schema2string(&i_schema);
}

std::string schema2string(const SCHEMA *i_schema)
{
    return (nullptr != i_schema) ? schema2string(&i_schema->Head, &i_schema->Body) : "";
}

std::string schema2string(const SCHEMA_HEAD *i_head, const SCHEMA_BODY *i_body)
{
    if(nullptr == i_head || nullptr == i_body) { return ""; }

    Json::Value result;
    Json::Value &head = result["head"];
    Json::Value &body = result["body"];
    {
        head["magic"]       = buffer_to_string(i_head->magic);
        head["version"]     = (Json::UInt64)i_head->version;
        head["sequence"]    = (Json::UInt64)i_head->sequence;
        head["type"]        = (Json::UInt64)i_head->type;
        head["pid"]         = (Json::UInt64)i_head->pid;
        head["ppid"]        = (Json::UInt64)i_head->ppid;
        head["time"]        = (Json::UInt64)i_head->time;
        head["event_type"]  = (Json::Int64)i_head->event_type;
        head["event_class"] = (Json::UInt64)i_head->event_class;
        head["data_len"]    = (Json::UInt64)i_head->data_len;
        head["reverse32"]   = (Json::UInt64)i_head->reverse32;
        head["reverse64"]   = (Json::UInt64)i_head->reverse64;
        head["rule_id"]     = (Json::UInt64)i_head->rule_id;

        const auto &name = g_event_type2name.find(i_head->event_type);
        head["event_name"] = (g_event_type2name.end() != name) ? name->second : "";
    }

    switch(i_head->event_type) {
        case EVENT_TYPE_FORK: {
            add_process_body(body, i_body->fork);
        } break;
        case EVENT_TYPE_CLONE:
        case EVENT_TYPE_EXEC:
        case EVENT_TYPE_EXEC_ENTRY: {
            add_process_body(body, i_body->exec);
        } break;
        case EVENT_TYPE_EXIT_GROUP: {
            add_process_body(body, i_body->exit);
        } break;
        case EVENT_TYPE_OPEN: {
            add_file_body(body, i_body->open);
        } break;
        case EVENT_TYPE_RENAME:
        case EVENT_TYPE_RENAMEAT:
        case EVENT_TYPE_RENAMEAT2:
        case EVENT_TYPE_VFS_RENAME: {
            add_file_body(body, i_body->rename);
        } break;
        case EVENT_TYPE_RCLOSE: {
            add_file_body(body, i_body->rclose);
        } break;
        case EVENT_TYPE_WCLOSE: {
            add_file_body(body, i_body->wclose);
        } break;
        case EVENT_TYPE_CONNECT: {
            const EVENT_NET_CONNECT &data = i_body->connect;
            body["protocol"]     = (Json::UInt64)data.protocol;
            body["src_port"]     = data.src_port;
            body["dst_port"]     = data.dst_port;
            body["src_addr"]     = buffer_to_string(data.src_addr);
            body["dst_addr"]     = buffer_to_string(data.dst_addr);
            body["is_ipv6"]      = (data.is_ipv6    != 0);
            body["is_inbound"]   = (data.is_inbound != 0);
            body["process_path"] = buffer_to_string(data.path_proc);
            body["parent_path"]  = buffer_to_string(data.path_pproc);
        } break;
        case EVENT_TYPE_RECVFROM:
        case EVENT_TYPE_RECVMSG:
        case EVENT_TYPE_SKB_CONSUME_UDP: {
            const EVENT_NET_DNS &data = i_body->dns;
            body["query_type"]    = (Json::UInt64)data.query_type;
            body["query_options"] = (Json::UInt64)data.query_options;
            body["query_result"]  = (Json::UInt64)data.query_result;
            body["query_name"]    = buffer_to_string(data.query_name);
            body["answers"]       = buffer_to_string(data.answers);
            body["process_path"]  = buffer_to_string(data.path_proc);
            body["parent_path"]   = buffer_to_string(data.path_pproc);
        } break;
        
        default: {} break;
    }

    Json::FastWriter writer; writer.omitEndingLineFeed();

    return writer.write(result);
}
