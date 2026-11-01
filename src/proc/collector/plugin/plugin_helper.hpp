
#ifndef plugin_public_hpp
#define plugin_public_hpp

#include "header/pch.hpp"

#include "base64/base64.hpp"
#include "cache/proc_cache_define.hpp"
#include "schema/schema_helper.hpp"
#include "file/file_api.hpp"
#include "jsoncpp/json/json.h"
#include "logger/logger.hpp"
#include "process/proc_info.hpp"
#include "string/string_api.hpp"
#if defined(OS_MACOS)
    #include "es/es_public_api.hpp"
#endif

#include "plugin/plugin.hpp"
#include "plugin/plugin_name.hpp"
#include "plugin/plugin_event.hpp"

#define PLUGIN_MESSAGE_HANDLER_ARGS uint64_t i_type, uint8_t *i_buf, size_t i_len

#define EVENT_COUNTER(i_code, o_head)                                               \
    do {                                                                            \
        static std::atomic_uint64_t counter{1};                                     \
        if(RC_OK == i_code) {                                                       \
            o_head->sequence = counter.fetch_add(1, std::memory_order_relaxed);     \
        }                                                                           \
    } while (false)                                                                 \

ret_code filter_match(filter::Filter &i_filter,
                      SCHEMA *i_msg,
                      const ProcInfoPtr &i_proc,
                      const ProcInfoPtr &i_pproc);

void copy_stat(struct stat   &i_stat, FileStat &o_stat);
void get_stat(const char     *i_path, FileStat &o_stat);
void get_stat(const uint8_t  *i_path, FileStat &o_stat);
void get_lstat(const char    *i_path, FileStat &o_stat);
void get_lstat(const uint8_t *i_path, FileStat &o_stat);

void set_header(EventType i_event, const ProcInfoPtr &i_proc_info, SCHEMA_HEAD *o_header);

ret_code event_register(const char *, EventRegisterArgs, message::MessageClient *);
ret_code event_unregister_all(const char *, EventRegisterArgs, message::MessageClient *);

bool try_get_proc_base_info(const pid_t &pid, ProcInfo &proc_info);



#endif /* plugin_public_hpp */
