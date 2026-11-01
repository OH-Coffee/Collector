
#include "logger/logger.hpp"
#include "system/sys_info.hpp"

#include "config/collector_config.hpp"
#include "engines/event_engine_factory.hpp"

#include "plugin/plugin_name.hpp"
#include "plugin/plugin_event.hpp"

#include "plugins-func/driver/driver.hpp"


static CollectorConfig *g_conf = &CollectorConfig::ins();

static std::list<std::string> get_engine_names();

Driver::Driver(message::MessageServer* i_server) : Plugin(PLUGIN_NAME_DRIVER, i_server)
{
    _plugin_state = STATE_UNKNOWN;
}

Driver::~Driver()
{
    _plugin_state = STATE_UNKNOWN;
}

ret_code Driver::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(RC_OK != (r_code = message_regist(CTRL_MSG_DRIVER_EVENT_REGISTER)))       { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_DRIVER_EVENT_UNREGISTER)))     { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_DRIVER_EVENT_UNREGISTER_ALL))) { break; }
        
        if(RC_OK != (r_code = message_regist(CTRL_MSG_DRIVER_PAUSE)))  { break; }
        if(RC_OK != (r_code = message_regist(CTRL_MSG_DRIVER_RESUME))) { break; }
        
        // 根据系统配置、体系架构创建合适的事件源引擎
        const auto names = get_engine_names();
        if(!names.empty()) {
            _engine_names.insert(names.begin(), names.end());
        }
        
        _engine_manager.create(_engine_names);
        if(_engine_manager.empty()) {
            r_code = RC_DRIVER_NO_ENGINE; break;
        }

        EngineManager::Names failed_names;
        
        // 事件源引擎初始化
        _engine_manager.control(EngineManager::ENGINES_INIT, failed_names);
        for(const auto &name : failed_names) {
            LOG_WARN("[Driver]\t[ NO] 事件源引擎：%s 初始化成功 -> 引擎将被移除", name.c_str());
        }
        _engine_manager.remove(failed_names);
        

        _plugin_state = (RC_OK == r_code) ? STATE_READY : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code Driver::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_READY != _plugin_state) { r_code = RC_PLUGIN_START_ERR; break; }
        {
            EngineManager::Names failed_names;
            
            _engine_manager.control(EngineManager::ENGINES_START, failed_names);
            if(failed_names.empty()) {
                r_code = RC_OK;
            }
        }
        _plugin_state = (RC_OK == r_code) ? STATE_RUNNING : STATE_FAILURE;
    } while (false);

    return r_code;
}

ret_code Driver::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _plugin_state  = STATE_STOPPED;

        this->exit(); this->wakeup();
        {
            EngineManager::Names failed_names;
            
            _engine_manager.control(EngineManager::ENGINES_STOP, failed_names);
            if(failed_names.empty()) {
                r_code = RC_OK;
            }
        }
        this->exit(); this->wakeup();

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while (false);

    return r_code;
}

ret_code Driver::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(STATE_STOPPED != _plugin_state) { r_code = this->stop(); }
        {
            EngineManager::Names failed_names;
            
            _engine_manager.control(EngineManager::ENGINES_UNINIT, failed_names);
            if(failed_names.empty()) {
                r_code = RC_OK;
            }
        }
        _plugin_state = STATE_UNKNOWN;

        r_code = (RC_UNKNOWN == r_code) ? RC_OK : r_code; break;
    } while(false);

    return r_code;
}

#if defined(OS_LINUX)
static bool is_tracepoint_ok()
{
    bool is_ok_tracepoint = true;

    do {
        auto os_arch = sys_machine();
        if(0 == strncmp("x86", os_arch.c_str(), 3)) {
            LOG_DEBUG("[Driver]\t[YES] 当前系统架构符合预期：%s；错误码：%d", os_arch.c_str(), RC_OK);
        } else {
            LOG_WARN( "[Driver]\t[ NO] 当前系统架构符合预期：%s -> tracepoint引擎无法启用 ；错误码：%d", os_arch.c_str(), RC_FAIL);
            is_ok_tracepoint = false; break;
        }

        auto sys_conf = sys_boot_config(30000);
        if(sys_conf.empty()) {
            LOG_WARN("[Driver]\t[ NO] 系统配置列表获取成功 -> tracepoint引擎无法启用；错误码：%d", RC_FAIL);
            is_ok_tracepoint = false; break;
        }

        std::list<std::string> conf_list = {
                "CONFIG_BPF",
                "CONFIG_BPF_EVENTS",
                "CONFIG_BPF_SYSCALL",
                "CONFIG_DEBUG_FS",
                "CONFIG_FTRACE",
                "CONFIG_FTRACE_SYSCALLS",
                "CONFIG_FUNCTION_TRACER",
                "CONFIG_TRACEPOINTS",
        };

        for(const auto &conf : conf_list) {
            auto it = sys_conf.find(conf);
            if(sys_conf.end() != it) {
                LOG_DIAGN("[Driver]\t[YES] 系统配置存在：%s；错误码：%d", conf.c_str(), RC_OK);
            } else {
                LOG_WARN( "[Driver]\t[ NO] 系统配置存在：%s -> tracepoint引擎无法启用；错误码：%d", conf.c_str(), RC_FAIL);
                is_ok_tracepoint = false; break;
            }

            if(0 == strncasecmp("y", it->second.c_str(), 1)) {
                LOG_DEBUG("[Driver]\t[YES] 系统配置启用：%s；错误码：%d", conf.c_str(), RC_OK);
            } else {
                LOG_WARN( "[Driver]\t[ NO] 系统配置启用：%s -> tracepoint引擎无法启用；错误码：%d", conf.c_str(), RC_FAIL);
                is_ok_tracepoint = false; break;
            }
        }
        if(!is_ok_tracepoint) { break; }

        is_ok_tracepoint = true;
    } while (false);
    
    return is_ok_tracepoint;
}

static bool is_kprobe_ok()
{
    bool is_ok_kprobe = true;

    do {
        // kprobe x86架构也能用，这里是因为作为tracepoint的下位替代所以屏蔽掉了
        auto os_arch = sys_machine();
        if(0 == strncmp("aarch64", os_arch.c_str(), 7)) {
            LOG_DEBUG("[Driver]\t[YES] 当前系统架构符合预期：%s； kprobe引擎启用, 错误码：%d", os_arch.c_str(), RC_OK);
        } else {
            LOG_WARN( "[Driver]\t[ NO] 当前系统架构符合预期：%s -> kprobe引擎无法启用 ；错误码：%d", os_arch.c_str(), RC_FAIL);
            is_ok_kprobe = false; break;
        }

        auto sys_conf = sys_boot_config(30000);
        if(sys_conf.empty()) {
            LOG_WARN("[Driver]\t[ NO] 系统配置列表获取成功 -> kprobe引擎无法启用；错误码：%d", RC_FAIL);
            is_ok_kprobe = false; break;
        }

        std::list<std::string> conf_list = {
                "CONFIG_BPF",
                "CONFIG_BPF_EVENTS",
                "CONFIG_BPF_SYSCALL",
                "CONFIG_DEBUG_FS",
                "CONFIG_FTRACE",
                "CONFIG_KPROBES",
                "CONFIG_HAVE_KPROBES",
                "CONFIG_KPROBE_EVENTS",
        };

        for(const auto &conf : conf_list) {
            auto it = sys_conf.find(conf);
            if(sys_conf.end() != it) {
                LOG_DIAGN("[Driver]\t[YES] 系统配置存在：%s；错误码：%d", conf.c_str(), RC_OK);
            } else {
                LOG_WARN( "[Driver]\t[ NO] 系统配置存在：%s -> kprobe引擎无法启用；错误码：%d", conf.c_str(), RC_FAIL);
                is_ok_kprobe = false; break;
            }

            if(0 == strncasecmp("y", it->second.c_str(), 1)) {
                LOG_DEBUG("[Driver]\t[YES] 系统配置启用：%s；错误码：%d", conf.c_str(), RC_OK);
            } else {
                LOG_WARN( "[Driver]\t[ NO] 系统配置启用：%s -> kprobe引擎无法启用；错误码：%d", conf.c_str(), RC_FAIL);
                is_ok_kprobe = false; break;
            }
        }
        if(!is_ok_kprobe) { break; }
    } while (false);

    return is_ok_kprobe;
}
#endif

std::list<std::string> get_engine_names()
{
    std::list<std::string> r_names;

    do {
        auto event_engines_in_conf = g_conf->get_strings< decltype(r_names) >("driver.engines");
        if(!event_engines_in_conf.empty()) {
            r_names = event_engines_in_conf;
        } else {
#if defined(OS_MACOS)
            r_names.emplace_back(ENGINE_NAME_ES);
#elif defined(OS_LINUX)
            r_names.emplace_back(ENGINE_NAME_FANOTIFY);

            if(is_tracepoint_ok()) {
                r_names.emplace_back(ENGINE_NAME_TRACEPOINT);
            } else {
                r_names.emplace_back(ENGINE_NAME_CONNECTOR);

                if (is_kprobe_ok()) {
                    r_names.emplace_back(ENGINE_NAME_KPROBE);
                }
            }

            r_names.emplace_back(ENGINE_NAME_UDDS);
#endif
        }
    } while (false);

    return r_names;
}

