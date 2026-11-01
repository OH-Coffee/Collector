
#include "header/pch.hpp"

#include "config/collector_config.hpp"
#include "plugin/plugin_helper.hpp"
#include "plugins-data/proc/plugin_proc.hpp"

#if defined(OS_MACOS)
    #include "es/es_client.hpp"
    #include "es/es_public_api.hpp"
#endif

static CollectorConfig *g_conf = &CollectorConfig::ins();

PluginProc::PluginProc(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_PROC, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

PluginProc::~PluginProc()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code PluginProc::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        EventObserver::set_name(PLUGIN_NAME_PROC);

        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_RELOAD))) { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_UNLOAD))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_PROC_PAUSE)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_PROC_RESUME))) { break; }

        auto thread_size            = g_conf->get_int64("plugin-proc.thread_size");
        auto thread_task_interval   = g_conf->get_int64("plugin-proc.thread_task_interval");
        auto thread_task_queue_size = g_conf->get_int64("plugin-proc.thread_task_queue_size");

        _threads.set_thread_size(thread_size);
        _threads.set_queue_size(thread_task_queue_size);
        _threads.set_task_interval(thread_task_interval);

        r_code = _threads.init();
        if(RC_OK != r_code) {
            r_code = RC_THREAD_POLL_INIT_ERR; break;
        }

        _plugin_state = (RC_OK == r_code) ? STATE_READY : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code PluginProc::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_READY != _plugin_state) { r_code = RC_PLUGIN_START_ERR; break; }
        do {
            r_code = _threads.start();
            if(RC_OK != r_code) {
                r_code = RC_THREAD_POLL_START_ERR; break;
            }
        } while (false);
        _plugin_state = (RC_OK == r_code) ? STATE_RUNNING : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code PluginProc::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _plugin_state  = STATE_STOPPED;

        this->exit(); this->wakeup();
        do {
            event_unregister_all("PluginProc", { EVENT_TYPE_ALL, this }, this);
            
            r_code = _threads.stop();
        } while (false);
        this->exit(); this->wakeup();

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while (false);

    return r_code;
}

ret_code PluginProc::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_STOPPED != _plugin_state) { this->stop(); }
        {
            r_code = _threads.uninit();
        }
        _plugin_state = STATE_UNKNOWN;

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while(false);

    return r_code;
}


