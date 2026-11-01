
#include "plugins-data/net/plugin_net.hpp"

ret_code PluginNet::msg_handler_for_plugin_pause(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    if(!_event_pause) {
        _event_pause = true;
        
        LOG_WARN("[PluginNet]\t[YES] 插件暂停 -> 该插件无法采集任何数据");
    }
    
    return RC_OK;
}

ret_code PluginNet::msg_handler_for_plugin_resume(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    if(_event_pause) {
        _event_pause = false;
        
        LOG_INFO("[PluginNet]\t[YES] 插件恢复");
    }
    
    return RC_OK;
}
