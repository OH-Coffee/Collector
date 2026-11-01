
#include "plugins-func/driver/driver.hpp"

ret_code Driver::message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    switch (i_type) {
        case CTRL_MSG_DRIVER_EVENT_REGISTER:       { r_code = msg_handler_for_event_register(i_type, i_buf, i_len);        } break;
        case CTRL_MSG_DRIVER_EVENT_UNREGISTER:     { r_code = msg_handler_for_event_unregister(i_type, i_buf, i_len);      } break;
        case CTRL_MSG_DRIVER_EVENT_UNREGISTER_ALL: { r_code = msg_handler_for_event_unregister_all(i_type, i_buf, i_len);  } break;
            
        case CTRL_MSG_DRIVER_PAUSE:  { r_code = msg_handler_for_plugin_pause(i_type, i_buf, i_len);  } break;
        case CTRL_MSG_DRIVER_RESUME: { r_code = msg_handler_for_plugin_resume(i_type, i_buf, i_len); } break;

        default: { r_code = RC_UNCONCERNED; } break;
    }

    return r_code;
}
