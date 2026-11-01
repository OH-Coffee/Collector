
#include "plugins-func/driver/driver.hpp"

ret_code Driver::msg_handler_for_plugin_resume(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(!_is_pause) { break; }
        {
            _engine_manager.control(EngineManager::ENGINES_RESUME);
        }
        _is_pause = false;
        
        LOG_INFO("[Driver]\t[YES] 插件恢复");
    } while (false);

    return r_code;
}

