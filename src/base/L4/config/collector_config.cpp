
#include "header/consts.hpp"

#include "logger/logger.hpp"
#include "config/collector_config.hpp"

#include "file/file_api.hpp"
#include "process/proc_info.hpp"
#include "string/string_api.hpp"

static ConfigManager *g_conf = &CollectorConfig::ins();

NEW_CONF_INIT_HANDLER(conf_init_log);
NEW_CONF_INIT_HANDLER(conf_init_rule);
NEW_CONF_INIT_HANDLER(conf_init_driver);
NEW_CONF_INIT_HANDLER(conf_init_controller);
NEW_CONF_INIT_HANDLER(conf_init_plugins);
NEW_CONF_INIT_HANDLER(conf_init_plugin_cache);
NEW_CONF_INIT_HANDLER(conf_init_plugin_proc);
NEW_CONF_INIT_HANDLER(conf_init_plugin_file);
NEW_CONF_INIT_HANDLER(conf_init_plugin_net);
// NEW_CONF_INIT_HANDLER(conf_init_monitor);
// NEW_CONF_INIT_HANDLER(conf_init_plugin_exception);

static const std::map<conf_init_handler, std::string> g_conf_init_handlers = {
    { conf_init_log,                "log"               },
    { conf_init_rule,               "rule"              },
    { conf_init_driver,             "driver"            },
    { conf_init_controller,         "controller"        },
    { conf_init_plugin_cache,       "plugin-cache"      },
    { conf_init_plugin_proc,        "plugin-proc"       },
    { conf_init_plugin_file,        "plugin-file"       },
    { conf_init_plugin_net,         "plugin-net"        },
    // { conf_init_plugin_dns,         "plugin-dns"        },
};


CollectorConfig& CollectorConfig::ins()
{
    static CollectorConfig config; return config;
}

ret_code CollectorConfig::load(const std::string &i_path)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        r_code = ConfigManager::load(i_path);
        
        for(auto &[handler, name] : g_conf_init_handlers) {
            auto val = handler(name, ConfigManager::_json_value);
            if(!val.isNull()) {
                ConfigManager::_json_value[name] = val;
            }
        }
    } while (false);
    
    return r_code;
}

NEW_CONF_INIT_HANDLER(conf_init_log)
{

    std::string dir = file_dir(proc_path(getpid()));

    g_conf->set_value<std::string>("log.path",  dir + "/../log/collector.log");
    g_conf->set_value<Json::Int>("log.level", LOG_LEVEL_INFO);
    g_conf->set_value<Json::Int>("log.max_file_size",   (5 * 1024 * 1024));
    g_conf->set_value<Json::Int>("log.max_buffer_size", (5 * 1024));
    
    g_conf->set_value<std::string>("log.cmd_log_path",  dir + "/../log/collector-cmd.log");
    g_conf->set_value<Json::Int>("log.cmd_log_level", LOG_LEVEL_TRACE);
    g_conf->set_value<Json::Int>("log.cmd_max_file_size",   (5 * 1024 * 1024));
    g_conf->set_value<Json::Int>("log.cmd_max_buffer_size", (5 * 1024));
    
    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_rule)
{
    std::string dir = file_dir(proc_path(getpid()));

    g_conf->set_value<std::string>("rule.path",  dir + "/../config/rule.xml");

    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_driver)
{
    g_conf->set_value<Json::Int>("driver.switch", SWITCH_ON);
    
    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_controller)
{
    g_conf->set_value<Json::Int>("controller.switch", SWITCH_ON);
    
    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_plugin_net)
{
    g_conf->set_value<Json::Int>("plugin-net.switch", SWITCH_ON);
    g_conf->set_value<Json::Int>("plugin-net.thread_size",             2);
    g_conf->set_value<Json::Int>("plugin-net.thread_task_interval",    1000);
    g_conf->set_value<Json::Int>("plugin-net.thread_task_queue_size",  1000);

    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_plugin_proc)
{
    g_conf->set_value<Json::Int>("plugin-proc.switch", SWITCH_ON);
    g_conf->set_value<Json::Int>("plugin-proc.thread_size",             1);
    g_conf->set_value<Json::Int>("plugin-proc.thread_task_interval",    1000);
    g_conf->set_value<Json::Int>("plugin-proc.thread_task_queue_size",  1000);

    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_plugin_file)
{
    g_conf->set_value<Json::Int>("plugin-file.switch", SWITCH_ON);
    g_conf->set_value<Json::Int>("plugin-file.thread_size",             2);
    g_conf->set_value<Json::Int>("plugin-file.thread_task_interval",    1000);
    g_conf->set_value<Json::Int>("plugin-file.thread_task_queue_size",  2000);

    return Json::Value{};
}

NEW_CONF_INIT_HANDLER(conf_init_plugin_cache)
{
    g_conf->set_value<Json::Int>("plugin-cache.switch", SWITCH_ON);

    g_conf->set_value<Json::Int>("plugin-cache.pid_cache_item_num",         100);
    g_conf->set_value<Json::Int>("plugin-cache.pid_cache_item_size",        256);
    g_conf->set_value<Json::Int>("plugin-cache.pid_cache_clear_interval",   1);

    g_conf->set_value<Json::Int>("plugin-cache.thread_size",             0);
    g_conf->set_value<Json::Int>("plugin-cache.thread_task_interval",    1000);
    g_conf->set_value<Json::Int>("plugin-cache.thread_task_queue_size",  1000);

    return Json::Value{};
}
