
#include "plugins-data/cache/plugin_cache.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

ret_code PluginCache::init_thread_pool()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        auto thread_size            = g_conf->get_int64("plugin-cache.thread_size");
        auto thread_task_interval   = g_conf->get_int64("plugin-cache.thread_task_interval");
        auto thread_task_queue_size = g_conf->get_int64("plugin-cache.thread_task_queue_size");

        _threads.set_thread_size(thread_size);
        _threads.set_queue_size(thread_task_queue_size);
        _threads.set_task_interval(thread_task_interval);

        r_code = _threads.init();
        if(RC_OK != r_code) {
            r_code = RC_THREAD_POLL_INIT_ERR; break;
        }
    } while (false);
    
    return r_code;
}
