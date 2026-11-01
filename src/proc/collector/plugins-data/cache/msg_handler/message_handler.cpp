
#include "plugins-data/cache/plugin_cache.hpp"

ret_code PluginCache::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_type) {
        case CTRL_MSG_PROXY_CONNTECT_OK:  { _upload_proc_cache = true;  r_code = RC_OK; } break;
        case CTRL_MSG_PROXY_CONNTECT_ERR: { _upload_proc_cache = false; r_code = RC_OK; } break;
            
        case CTRL_MSG_PLUGIN_CACHE_PAUSE:  { r_code = msg_handler_for_plugin_pause(i_type, i_buf, i_len);  } break;
        case CTRL_MSG_PLUGIN_CACHE_RESUME: { r_code = msg_handler_for_plugin_resume(i_type, i_buf, i_len); } break;
            
        case CTRL_MSG_PLUGIN_CACHE_EVENT_RELOAD: {
            r_code = msg_handler_for_event_reload(i_type, i_buf, i_len);
        } break;
            
        case CTRL_MSG_PLUGIN_CACHE_EVENT_UNLOAD: {
            r_code = msg_handler_for_event_unload(i_type, i_buf, i_len);
        } break;

        default: r_code = RC_UNCONCERNED; break;
    }

    return r_code;
}
