
#include <iostream>

#include "header/pch.hpp"
#include "header/consts.hpp"

#include "plugin/plugin_name.hpp"
#include "plugin/plugin_event.hpp"
#include "plugin/plugin_helper.hpp"

#include "plugins-func/proxy/proxy.hpp"

Proxy::Proxy(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_PROXY, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

Proxy::~Proxy()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code Proxy::init()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        r_code = message_regist(MessageType::DATA_MSG_SEND_TO_PROXY);
        r_code = message_regist(MessageType::CTRL_MSG_SEND_TO_SERVER);
        r_code = message_regist(MessageType::CTRL_MSG_PROXY_DISCONNTECT_ONCE);
        
        _plugin_state = STATE_READY;
    } while (false);
    
    return r_code;
}

ret_code Proxy::start()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        r_code = RC_OK;
    } while (false);
    
    return r_code;
}

ret_code Proxy::stop()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        this->exit();
        this->wakeup();
        
        _plugin_state = STATE_STOPPED;
        
        r_code = RC_OK;
    } while (false);

    
    return r_code;
}

ret_code Proxy::uninit()
{
    ret_code r_code = RC_UNKNOWN;
    
    r_code = this->stop();
    
    return r_code;
}

ret_code Proxy::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(RC_OK == this->is_exit()) { r_code = RC_EXIT; break; }
        
        switch (i_type) {
            case CTRL_MSG_SEND_TO_SERVER:
            case CTRL_MSG_PROXY_DISCONNTECT_ONCE: { } break;
            case DATA_MSG_SEND_TO_PROXY: {
                std::cout << schema2string(reinterpret_cast<SCHEMA *>(i_buf)) << std::endl;
            } break;
                
            default: r_code = RC_UNCONCERNED; break;
        }
    } while (false);
    
    return r_code;
}

