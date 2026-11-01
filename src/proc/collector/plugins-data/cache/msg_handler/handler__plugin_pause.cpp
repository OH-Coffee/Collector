
#include "plugins-data/cache/plugin_cache.hpp"

ret_code PluginCache::msg_handler_for_plugin_pause(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { break; }
        
        LOG_WARN("[PluginCache]\t[YES] 插件暂停 -> 该插件无法采集任何数据");
        
        _is_pause = true;
    } while (false);

    return r_code;
}
