
#include "plugins-data/cache/plugin_cache.hpp"

ret_code PluginCache::msg_handler_for_event_unload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = event_unregister_all("PluginCache", { EVENT_TYPE_ALL, this }, this);
    } while (false);

    return r_code;
}
