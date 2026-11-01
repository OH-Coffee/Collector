#include "plugins-func/controller/controller.hpp"

ret_code Controller::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_type) {
        case CTRL_MSG_RECV_FROM_SERVER: {} break;

        case CTRL_MSG_CONTROLLER_STOP: { r_code = msg_handler_for_controller_stop(i_type, i_buf, i_len);      } break;
        
        case CTRL_MSG_CONTROLLER_STATE_UPLOAD:  {} break;
        case CTRL_MSG_PROXY_CONNTECT_OK:        {} break;
        case CTRL_MSG_PROXY_CONNTECT_ERR:       {} break;
        case CTRL_MSG_MESSAGE_CENTER_SPEEDUP:
        case CTRL_MSG_MESSAGE_CENTER_SLOWDOWN:
        case CTRL_MSG_MESSAGE_CENTER_SPEED_CHANGE:  {} break;
        case INFO_MSG_EVENT_ENGINES_RECONSTRUCT:    {} break;

        default: r_code = RC_UNCONCERNED; break;
    }

    return r_code;
}
