
#include "plugins-data/cache/plugin_cache.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

PluginCache::PluginCache(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_CACHE, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

PluginCache::~PluginCache()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code PluginCache::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        EventObserver::set_name(PLUGIN_NAME_CACHE);
        EventObserver::set_nice(EVENT_OBSERVER_NICE_HIGH);
        
        if(RC_OK != (r_code = init_thread_pool()))                          { break; }
        if(RC_OK != (r_code = thread_for_pid_cache_clear()))                { break; }
        if(RC_OK != (r_code = thread_for_proc_cache_upload()))              { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PROXY_CONNTECT_OK)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PROXY_CONNTECT_ERR))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_CACHE_PAUSE)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_CACHE_RESUME))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_CACHE_EVENT_RELOAD)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_CACHE_EVENT_UNLOAD)))  { break; }
        
        _plugin_state = (RC_OK == r_code) ? STATE_READY : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code PluginCache::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = _threads.start();
        if(RC_OK != r_code) {
            r_code = RC_THREAD_POLL_START_ERR; break;
        }
        
        r_code = this->send_syn(CTRL_MSG_PLUGIN_CACHE_EVENT_RELOAD, nullptr, 0);
        if(RC_OK != r_code) {
            break;
        }

        _plugin_state = (RC_OK == r_code) ? STATE_RUNNING : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code PluginCache::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _plugin_state = STATE_STOPPED;

        this->exit(); this->wakeup();
        do {
            this->send_syn(CTRL_MSG_PLUGIN_CACHE_EVENT_UNLOAD, nullptr, 0);
            
            _threads.stop();
            _pid_cache.clear_all_item();
        } while (false);
        this->exit(); this->wakeup();

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while (false);

    return r_code;
}

ret_code PluginCache::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_STOPPED != _plugin_state) { this->stop(); }
        {
            _plugin_state = STATE_UNKNOWN;

            _pid_cache.del_observer(PID_CACHE_STAGE_INSERT, this);
            _pid_cache.del_observer(PID_CACHE_STAGE_DELETE, this);

            r_code = _threads.uninit();
        }
        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while(false);

    return r_code;
}


