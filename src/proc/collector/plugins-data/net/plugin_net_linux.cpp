
#include "header/pch.hpp"
#include "config/collector_config.hpp"
#include "plugins-data/net/plugin_net.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

PluginNet::PluginNet(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_NET, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

PluginNet::~PluginNet()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code PluginNet::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
#ifdef ENABLE_NSS_EXTENSION
        if(_ex_controller == nullptr) {
            _ex_controller = NEW_NSS_EXTENSION_CONTROLLER(ID_NSS_EXTENSION, NAME_NSS_EXTENSION);
        }
        if(_ex_controller == nullptr) { r_code = RC_PLUGIN_NET_NOT_INIT; break; }
#endif
        EventObserver::set_name(PLUGIN_NAME_NET);

        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_RELOAD))) { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONF_UNLOAD))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_NET_PAUSE)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_PLUGIN_NET_RESUME))) { break; }

        auto thread_size            = g_conf->get_int64("plugin-net.thread_size");
        auto thread_task_interval   = g_conf->get_int64("plugin-net.thread_task_interval");
        auto thread_task_queue_size = g_conf->get_int64("plugin-net.thread_task_queue_size");

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

ret_code PluginNet::start()
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

ret_code PluginNet::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _plugin_state  = STATE_STOPPED;

        this->exit(); this->wakeup();
        {
            r_code = event_unregister_all("PluginNet", { EVENT_TYPE_ALL, this }, this);
            r_code = _threads.stop();
        }
        this->exit(); this->wakeup();

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while (false);

    return r_code;
}

ret_code PluginNet::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_STOPPED != _plugin_state) { r_code = this->stop(); }
        {
            r_code = _threads.uninit();
        }
        _plugin_state = STATE_UNKNOWN;

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while(false);

    return r_code;
}

