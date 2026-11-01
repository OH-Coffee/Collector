
#include "rule/loader.hpp"
#include "filter/filter.hpp"
#include "config/collector_config.hpp"

#include "plugins-func/controller/controller.hpp"

#include "plugins-func/driver/driver.hpp"
#include "plugins-func/proxy/proxy.hpp"

#include "plugins-data/cache/plugin_cache.hpp"
#include "plugins-data/proc/plugin_proc.hpp"
#include "plugins-data/file/plugin_file.hpp"
#include "plugins-data/net/plugin_net.hpp"


static auto *g_conf = &CollectorConfig::ins();

Controller::Controller() : PluginManager(PLUGIN_NAME_CONTROLLER, &_message_center)
{
    _plugin_state = STATE_UNKNOWN;
}

Controller::~Controller()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code Controller::init()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Controller] 控制器构造开始");
    do {
        if(RC_OK != (r_code = _message_center.init()))  { break; }
        if(RC_OK != (r_code = _message_center.start())) { break; }

        if(RC_OK != (r_code = message_regist(CTRL_MSG_CONTROLLER_STOP))) { break; }
        
        r_code = init_plugins();
    } while (false);
    LOG_INFO("[Controller] 控制器构造完成，错误码：%d", r_code);
    
    return r_code;
}

ret_code Controller::init_plugins()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        try {
#define PLUGIN_ADD_WITH_SWITCH(i_switch, i_class)                  \
            do { if(SWITCH_ON == g_conf->get_int64(i_switch)) {    \
                _plugins.push_back(new i_class(&_message_center)); \
            } } while(false)                                       \
            
            PLUGIN_ADD_WITH_SWITCH("driver.switch",         Driver);
            PLUGIN_ADD_WITH_SWITCH("plugin-cache.switch",   PluginCache);
            PLUGIN_ADD_WITH_SWITCH("plugin-proc.switch",    PluginProc);
            PLUGIN_ADD_WITH_SWITCH("plugin-file.switch",    PluginFile);
            PLUGIN_ADD_WITH_SWITCH("plugin-net.switch",     PluginNet);
            
            // PLUGIN_ADD_WITH_SWITCH("monitor.switch",        PluginMonitor);
            // PLUGIN_ADD_WITH_SWITCH("exception.switch",      PlguinException);
            
            // _plugins.push_back(new Wrapper(&_message_center));
            _plugins.push_back(new Proxy(&_message_center));
#undef PLUGIN_ADD_WITH_SWITCH
        } catch (...) {
            _plugin_state = STATE_FAILURE;
            r_code = RC_CONTROLLER_PLUGIN_ALLOC_ERR; break;
        }
        
        r_code = _message_center.add(this);
        
        for(auto it = _plugins.begin(); it != _plugins.end(); ) {
            Plugin *plugin = (*it);
            
            ret_code r_init = plugin->init();
            if(r_init == RC_OK) {
                LOG_INFO( "[Controller]\t[YES] 插件构造: %s；错误码：%d", plugin->name().c_str(), r_init);
            } else {
                LOG_ERROR("[Controller]\t[ NO] 插件构造: %s -> 插件将被移除；错误码：%d", plugin->name().c_str(), r_init);
                it = _plugins.erase(it); continue;
            }
            
            r_code = _message_center.add(plugin);
            if(r_code != RC_OK) {
                LOG_ERROR("[Controller]\t[ NO] 插件：%s 向消息中心注册，错误码：%d", plugin->name().c_str(), r_init);
            }
            
            ++it;
        }
        
        _plugin_state = STATE_READY; r_code = RC_OK;
    } while (false);
    
    return r_code;
}


ret_code Controller::start()
{
    ret_code r_code = RC_UNKNOWN;

    LOG_INFO("[Controller] 控制器运行中...");

    rule::Loader loader;
    r_code = loader.load_by_path(g_conf->get_string("rule.path"), rule::LoadMode::MODE_RELOAD);
    if(RC_OK != r_code) {
        LOG_ERROR("[Controller]\t[ NO] 规则加载，错误码：%d", r_code);
    }

    r_code = global::filter().load(loader);
    if(RC_OK != r_code) {
        LOG_ERROR("[Controller]\t[ NO] 过滤器加载，错误码：%d", r_code);
    }
    
    for(const auto &it : _plugins) {
        ret_code r_start = it->start();
        if(r_start == RC_OK) {
            LOG_INFO( "[Controller]\t[YES] 插件启动: %s，错误码：%d", it->name().c_str(), r_start);
        } else {
            LOG_ERROR("[Controller]\t[ NO] 插件启动: %s，错误码：%d", it->name().c_str(), r_start);
        }
    }
    
    std::vector<EventType> events = {
        EVENT_TYPE_CONNECT,
    };
    // for(const auto &[event, rule] : loader.get_filter_rule_groups()) {
    //     events.push_back(static_cast<EventType>(event));
    // }

    r_code = MessageClient::send_syn(CTRL_MSG_CONF_RELOAD,
                                     reinterpret_cast<uint8_t *>(events.data()),
                                     sizeof(EventType) * events.size());
    if(RC_OK == r_code) {
        LOG_TRACE("[Controller]\t[YES] 事件注册消息发送完成；错误码：%d", r_code);
    } else {
        LOG_ERROR("[Controller]\t[ NO] 事件注册消息发送完成 -> 可能部分事件并未注册成功；错误码：%d", r_code);
    }


    while(RC_OK != this->is_exit()) {
        this->sleep_ms(1000 * 3);
    }
    
    LOG_INFO("[Controller] 控制器运行结束，错误码：%d", r_code = (RC_OK == this->is_exit()) ? RC_EXIT : r_code);
    
    return RC_OK;
}
ret_code Controller::stop()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Controller] 控制器停止开始");
    do {
        _plugin_state  = STATE_STOPPED;

        this->exit(); this->wakeup();
        {
            for(const auto &it : _plugins) {
                ret_code r_stop = it->stop();
                if(RC_OK == r_stop) {
                    LOG_INFO( "[Controller]\t[YES] 插件停止: %s，错误码：%d", it->name().c_str(), r_stop);
                } else {
                    LOG_ERROR("[Controller]\t[ NO] 插件停止: %s，错误码：%d", it->name().c_str(), r_stop);
                }
            }
        }
        this->exit(); this->wakeup(); r_code = RC_OK; break;
    } while (false);
    LOG_INFO("[Controller] 控制器停止完成，错误码：%d", r_code);
    
    return r_code;
}
ret_code Controller::uninit()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Controller] 控制器析构开始");
    do {
        if(STATE_STOPPED != _plugin_state) { this->stop(); }
        {
            _message_center.stop();
            _message_center.uninit();

            for(auto &plugin : _plugins) {
                ret_code err_code = plugin->uninit();
                if(err_code == RC_OK) {
                    LOG_INFO( "[Controller]\t[YES] 插件析构: %s，错误码：%d", plugin->name().c_str(), err_code);
                } else {
                    LOG_ERROR("[Controller]\t[ NO] 插件析构: %s，错误码：%d", plugin->name().c_str(), err_code);
                }
            }
        }
        _plugin_state = STATE_UNKNOWN;

        r_code = (RC_OK == r_code) ? RC_OK : r_code; break;
    } while (false);
    LOG_INFO("[Controller] 控制器析构完成，错误码：%d", r_code);

    return r_code;
}