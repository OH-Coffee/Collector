
#ifndef engine_manager_h
#define engine_manager_h

#include <map>
#include <set>

#include "logger/logger.hpp"
#include "plugin/plugin_event.hpp"
#include "engines/event_engine.hpp"

class EngineManager
{
public:
     EngineManager() = default;
    ~EngineManager() = default;
public:
    EngineManager(EngineManager &&)      = delete;
    EngineManager(const EngineManager &) = delete;
    
    EngineManager& operator= (EngineManager &&)      = delete;
    EngineManager& operator= (const EngineManager &) = delete;
public:
    using Names = std::set<std::string>;
    
    enum ControlCommand : uint64_t {
        ENGINES_INIT,
        ENGINES_START,
        ENGINES_STOP,
        ENGINES_UNINIT,
        
        ENGINES_PAUSE,
        ENGINES_RESUME,
        
        ENGINES_EVENT_REGISTER,
        ENGINES_EVENT_UNREGISTER,
        ENGINES_EVENT_UNREGISTER_ALL,
    };
private:
    const std::map<ControlCommand, std::string> _cmd2string{
        { ENGINES_INIT,                  "ENGINES_INIT"                   },
        { ENGINES_START,                 "ENGINES_START"                  },
        { ENGINES_STOP,                  "ENGINES_STOP"                   },
        { ENGINES_UNINIT,                "ENGINES_UNINIT"                 },
        
        { ENGINES_PAUSE,                 "ENGINES_PAUSE"                  },
        { ENGINES_RESUME,                "ENGINES_RESUME"                 },
        
        { ENGINES_EVENT_REGISTER,        "ENGINES_EVENT_REGISTER"         },
        { ENGINES_EVENT_UNREGISTER,      "ENGINES_EVENT_UNREGISTER"       },
        { ENGINES_EVENT_UNREGISTER_ALL,  "ENGINES_EVENT_UNREGISTER_ALL"   },
    };
    
public:
    inline bool create(const Names &i_engine_names) {
        std::lock_guard<std::mutex> guard(_mutex_for_engines); return this->_create(i_engine_names);
    }
    inline void remove(const Names &i_engine_names) {
        std::lock_guard<std::mutex> guard(_mutex_for_engines); return this->_remove(i_engine_names);
    }
public:
    inline void clear() {
        std::lock_guard<std::mutex> guard(_mutex_for_engines); return this->_clear();
    }
    inline bool empty() {
        std::lock_guard<std::mutex> guard(_mutex_for_engines); return _engines.empty();
    }
public:
#define LOCK_GUARD() std::lock_guard<std::mutex> guard(_mutex_for_engines)
    
    // 有一个引擎执行指令成功，此control()系列函数返回成功，执行指令失败的引擎名称经参数输出
    inline ret_code control(ControlCommand i_cmd) {
        LOCK_GUARD(); Names _; return this->_control(i_cmd, _);
    }
    inline ret_code control(ControlCommand i_cmd, Names &o_names) {
        LOCK_GUARD(); return this->_control(i_cmd, o_names);
    }
    inline ret_code control(ControlCommand i_cmd, const EventRegisterArgs &i_args, Names &o_names) {
        LOCK_GUARD(); return this->_control(i_cmd, i_args, o_names);
    }
    
#undef LOCK
    
private:
    bool _create(const Names &);
    void _remove(const Names &);
    void _clear();
private:
    ret_code _control(ControlCommand, Names &);
    ret_code _control(ControlCommand, const EventRegisterArgs &, Names &);
private:
    const char *cmd2string(ControlCommand i_cmd);
private:
    std::map<std::string, EventEnginePtr> _engines;
    std::mutex                            _mutex_for_engines;
};

#endif /* engine_manager_h */
