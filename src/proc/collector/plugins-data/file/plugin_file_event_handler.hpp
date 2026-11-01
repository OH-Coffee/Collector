
#ifndef plugin_file_event_handler_h
#define plugin_file_event_handler_h

#include "cache/proc_cache_define.hpp"
#include "string/string_api.hpp"

#include "plugin/plugin_helper.hpp"
#include "plugins-data/file/plugin_file.hpp"

#define GET_ARRAY_NUM(i_array) ( sizeof(i_array) / sizeof(i_array[0]) )

#if defined(OS_MACOS)
    ret_code event_handler_for_open(const es_message_t *,   SCHEMA *);
    ret_code event_handler_for_close(const es_message_t *,  SCHEMA *);
    ret_code event_handler_for_create(const es_message_t *, SCHEMA *);
    ret_code event_handler_for_rename(const es_message_t *, SCHEMA *);
#elif defined(OS_LINUX)
    ret_code event_handler_for_rename(const EventPtr &,      SCHEMA *);
    ret_code event_handler_for_vfs_rename(const EventPtr &,  SCHEMA *);
    ret_code event_handler_for_close_read(const EventPtr &,  SCHEMA *);
    ret_code event_handler_for_close_write(const EventPtr &, SCHEMA *);
#endif

inline bool is_valid_type(mode_t i_mode, const mode_t i_types[], const int i_types_num)
{
    const auto type = (S_IFMT & i_mode);
    
    for (int i = 0; i < i_types_num; i++) {
        if (type == i_types[i]) { return true; }
    }
    
    return false;
}

const std::list<std::string> &get_file_rclose_event_inside_name();
const std::list<std::string> &get_file_wclose_event_inside_name();

#endif /* plugin_file_event_handler_h */
