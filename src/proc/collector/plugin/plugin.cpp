
#include "plugin/plugin.hpp"
#include "plugin/plugin_event.hpp"

namespace global
{
    filter::Filter& filter() { static filter::Filter filter; return filter; }
}

ret_code Plugin::msg_send(const message::MESSAGE_PTR & i_msg)
{
    ret_code r_code = RC_UNKNOWN;
    
    if(i_msg->type > CTRL_MSG_UNKNOWN && i_msg->type < CTRL_MSG_MAXIMUM) {
        r_code = message::MessageClient::send_asyn(i_msg, message::NICE_HIGH);
    } else {
        r_code = message::MessageClient::send_asyn(i_msg);
    }
    
    return r_code;
}

ret_code Plugin::msg_send(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    
    if(i_type > CTRL_MSG_UNKNOWN && i_type < CTRL_MSG_MAXIMUM) {
        r_code = message::MessageClient::send(i_type, i_buf, i_len, message::NICE_HIGH);
    } else {
        r_code = message::MessageClient::send(i_type, i_buf, i_len);
    }
    
    return r_code;
}

