
#include "plugins-data/proc/plugin_proc.hpp"


static const std::set<EventType> g_events = {
    EVENT_TYPE_EXEC,
};


ret_code PluginProc::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_type) {
        case CTRL_MSG_CONF_RELOAD: { r_code = msg_handler_for_event_reload(i_type, i_buf, i_len); } break;
        case CTRL_MSG_CONF_UNLOAD: { r_code = msg_handler_for_event_unload(i_type, i_buf, i_len); } break;
            
        case CTRL_MSG_PLUGIN_PROC_PAUSE:  { r_code = msg_handler_for_plugin_pause(i_type, i_buf, i_len);  } break;
        case CTRL_MSG_PLUGIN_PROC_RESUME: { r_code = msg_handler_for_plugin_resume(i_type, i_buf, i_len); } break;
            
        default: r_code = RC_UNCONCERNED; break;
    }

    return r_code;
}

ret_code PluginProc::msg_handler_for_event_reload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    do {
        if(RC_OK != (r_code = msg_handler_for_event_unload(i_type, i_buf, i_len))) { break; }

        for(size_t i = 0; i < (i_len / sizeof(EventType)); i++) {
            auto event = reinterpret_cast<EventType *>(i_buf)[i];

            if(g_events.end() == g_events.find(event)) {
                continue;
            }

            r_code = event_register("PluginProc", { event, this }, this);
        }
    } while (false);

    return r_code;
}

ret_code PluginProc::msg_handler_for_event_unload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = event_unregister_all("PluginProc", { EVENT_TYPE_ALL, this }, this);
    } while (false);

    return r_code;
}

ret_code PluginProc::msg_handler_for_plugin_pause(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { break; }
        
        LOG_WARN("[PluginProc]\t[YES] 插件暂停 -> 该插件无法采集任何数据");
        
        _is_pause = true;
    } while (false);

    return r_code;
}

ret_code PluginProc::msg_handler_for_plugin_resume(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(!_is_pause) { break; }
        
        LOG_INFO("[PluginProc]\t[YES] 插件恢复");
        
        _is_pause = false;
    } while (false);

    return r_code;
}
