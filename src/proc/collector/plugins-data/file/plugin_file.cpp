
#include "header/pch.hpp"
#include "config/collector_config.hpp"

#include "plugin/plugin_helper.hpp"
#include "plugins-data/file/plugin_file.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

PluginFile::PluginFile(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_FILE, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

PluginFile::~PluginFile()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code PluginFile::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        EventObserver::set_name(PLUGIN_NAME_FILE);

        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_RELOAD))) { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_UNLOAD))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_FILE_PAUSE)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_FILE_RESUME))) { break; }

        auto thread_size            = g_conf->get_int64("plugin-file.thread_size");
        auto thread_task_interval   = g_conf->get_int64("plugin-file.thread_task_interval");
        auto thread_task_queue_size = g_conf->get_int64("plugin-file.thread_task_queue_size");
        
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

ret_code PluginFile::start()
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

ret_code PluginFile::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _plugin_state  = STATE_STOPPED;

        this->exit(); this->wakeup();
        do {
            event_unregister_all("PluginFile", { EVENT_TYPE_ALL, this }, this);
            r_code = _threads.stop();
        } while (false);
        this->exit(); this->wakeup();

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while (false);

    return r_code;
}

ret_code PluginFile::uninit()
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
