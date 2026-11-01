
#include "plugins-func/driver/driver.hpp"

ret_code Driver::msg_handler_for_event_unregister(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_buf || i_len <= 0) { r_code = RC_ARG_ERR;           break; }
        if(_engine_manager.empty())        { r_code = RC_DRIVER_NO_ENGINE;  break; }

        auto event_args_ptr = (EventRegisterArgs *)i_buf;
        if(nullptr == event_args_ptr->observer) {
            r_code = RC_ARG_ERR; break;
        }
        
        EngineManager::Names failed_names;
        
        r_code = _engine_manager.control(EngineManager::ENGINES_EVENT_UNREGISTER, *event_args_ptr, failed_names);
        if(failed_names.size() == _engine_names.size()) {
            LOG_WARN("[Driver]\t[ NO] 订阅者：%s 事件：%d 注销成功 -> 该订阅者可能并未订阅任何事件；错误码：%d",
                      event_args_ptr->observer->get_name().c_str(),
                      event_args_ptr->event,
                      r_code);
        } else if(RC_OK != r_code) {
            LOG_ERROR("[Driver]\t[ NO] 订阅者：%s 事件：%d 注销成功；错误码：%d",
                      event_args_ptr->observer->get_name().c_str(),
                      event_args_ptr->event,
                      r_code);
        }
    } while (false);

    return r_code;
}
