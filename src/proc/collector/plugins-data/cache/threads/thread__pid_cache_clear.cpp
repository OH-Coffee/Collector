
#include "plugins-data/cache/plugin_cache.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

ret_code PluginCache::thread_for_pid_cache_clear()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        uint32_t pid_cache_num  = g_conf->get_uint32("plugin-cache.pid_cache_item_num");
        uint32_t pid_cache_size = g_conf->get_uint32("plugin-cache.pid_cache_item_size");
        
        _pid_cache.set_capacity(pid_cache_num, pid_cache_size);
        
        r_code = _pid_cache.add_observer(PID_CACHE_STAGE_DELETE, this);
        if(RC_OK != r_code) {
            break;
        }
        
        std::thread thread_for_pid_recorder([this](){
            const uint32_t sleep_time = g_conf->get_uint32("plugin-cache.pid_cache_clear_interval");
            while (RC_OK != this->is_exit()) {
                this->sleep_ms(sleep_time * 1000);
                this->_pid_cache.clear_full_item();
            }
        }); thread_for_pid_recorder.detach();
    } while (false);
    
    return r_code;
}
