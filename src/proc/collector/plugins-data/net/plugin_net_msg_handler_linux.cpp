
#include "plugins-data/net/plugin_net.hpp"


static const std::set<EventType> g_events =  {
    EVENT_TYPE_RECVMSG,
    EVENT_TYPE_RECVFROM,
    EVENT_TYPE_SKB_CONSUME_UDP,
    EVENT_TYPE_CONNECT,
};


ret_code PluginNet::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_type) {
        case CTRL_MSG_CONF_RELOAD: { r_code = msg_handler_for_event_reload(i_type, i_buf, i_len); } break;
        case CTRL_MSG_CONF_UNLOAD: { r_code = msg_handler_for_event_unload(i_type, i_buf, i_len); } break;
            
        case CTRL_MSG_PLUGIN_NET_PAUSE:  { r_code = msg_handler_for_plugin_pause(i_type, i_buf, i_len);  } break;
        case CTRL_MSG_PLUGIN_NET_RESUME: { r_code = msg_handler_for_plugin_resume(i_type, i_buf, i_len); } break;
            
        default: r_code = RC_UNCONCERNED; break;
    }

    return r_code;
}

ret_code PluginNet::msg_handler_for_event_reload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    do {
        if(RC_OK != (r_code = msg_handler_for_event_unload(i_type, i_buf, i_len))) { break; }

        for(size_t i = 0; i < (i_len / sizeof(EventType)); i++) {
            auto event = reinterpret_cast<EventType *>(i_buf)[i];

            if(g_events.end() == g_events.find(event)) {
                continue;
            }

            r_code = event_register("PluginNet", { event, this }, this);
        }
    } while (false);

    return r_code;
}

ret_code PluginNet::msg_handler_for_event_unload(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
#ifdef ENABLE_NSS_EXTENSION
        if(SWITCH_ON == _plugin_switch) { _ex_controller->unregist(); }
#endif
        r_code = event_unregister_all("PluginNet", { EVENT_TYPE_ALL, this }, this);

        _plugin_switch = SWITCH_OFF;
    } while (false);

    return r_code;
}
