
#include "plugins-data/cache/plugin_cache.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

ret_code PluginCache::thread_for_proc_cache_upload()
{
    ret_code r_code = ProcCache::ins().init();
    
    return r_code;
}
