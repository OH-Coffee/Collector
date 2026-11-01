
#include "plugins-func/driver/driver.hpp"

ret_code Driver::msg_handler_for_plugin_pause(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { break; }
        {
            _engine_manager.control(EngineManager::ENGINES_PAUSE);
        }
        _is_pause = true;
        
        LOG_WARN("[Driver]\t[YES] 插件暂停 -> 该插件无法采集任何数据");
    } while (false);

    return r_code;
}
