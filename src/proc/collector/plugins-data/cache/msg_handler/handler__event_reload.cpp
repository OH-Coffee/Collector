
#include "plugins-data/cache/plugin_cache.hpp"

ret_code PluginCache::msg_handler_for_event_reload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    do {
        if(RC_OK != (r_code = msg_handler_for_event_unload(i_type, i_buf, i_len))) { break; }

        static const std::list<EventType> events = {
        #if defined(OS_MACOS)
            EVENT_TYPE_FORK,
            EVENT_TYPE_EXEC,
        #elif defined(OS_LINUX)
            EVENT_TYPE_CLONE,
            EVENT_TYPE_EXEC,
            EVENT_TYPE_EXEC_ENTRY,
            EVENT_TYPE_EXIT_GROUP,
        #endif
        };
        
        for(const auto event: events) {
            r_code = (RC_OK == event_register("Cache", { event, this }, this)) ? RC_OK : r_code;
        }
    } while (false);

    return r_code;
}
