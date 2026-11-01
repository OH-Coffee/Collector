
#include <mutex>

#include "engine_manager.hpp"
#include "engines/event_engine_factory.hpp"


bool EngineManager::_create(const Names &i_names)
{
    bool r_code = true;
    
    this->_clear();
    
    for(const auto &name : i_names) {
        auto engine = EventEngineFactory::create(name);
        
        if(nullptr != engine) {
            _engines.insert( { engine->name(), engine } );
        } else {
            r_code = false;
        }
    }
    
    return r_code;
}

void EngineManager::_remove(const Names &i_names)
{
    for(const auto &name : i_names) {
        auto it = _engines.find(name);
        
        if(_engines.end() != it) {
            auto &engine = it->second;
            
            engine->stop();
            engine->uninit();
            
            _engines.erase(it);
        }
    }
}

void EngineManager::_clear()
{
    Names failed_names;
    
    this->_control(ENGINES_STOP, failed_names);
    this->_control(ENGINES_UNINIT, failed_names);
    
    _engines.clear();
}

ret_code EngineManager::_control(ControlCommand i_cmd, Names &o_names)
{
    ret_code r_code = RC_UNKNOWN;
    
    for(const auto &[name, engine] : _engines) {
        ret_code r_action = RC_UNKNOWN;
        
        switch (i_cmd) {
            case ENGINES_INIT:   { r_action = engine->init();    } break;
            case ENGINES_START:  { r_action = engine->start();   } break;
            case ENGINES_STOP:   { r_action = engine->stop();    } break;
            case ENGINES_UNINIT: { r_action = engine->uninit();  } break;
                
            case ENGINES_PAUSE:  { r_action = RC_OK; engine->pause();   } break;
            case ENGINES_RESUME: { r_action = RC_OK; engine->resume();  } break;
            default: break;
        }
        
        if(RC_OK == r_action) {
            r_code = (r_action == RC_OK) ? r_action : r_code;
            LOG_INFO("[Driver]\t[YES] 事件源引擎：%s -> 执行动作：%s；错误码：%d", name.c_str(), cmd2string(i_cmd), r_action);
        } else {
            o_names.insert(name);
            
            LOG_WARN("[Driver]\t[ NO] 事件源引擎：%s -> 执行动作：%s；错误码：%d", name.c_str(), cmd2string(i_cmd), r_action);
        }
    }
    
    return r_code;
}

ret_code EngineManager::_control(ControlCommand i_cmd, const EventRegisterArgs &i_args, Names &o_names)
{
    ret_code r_code = RC_UNCONCERNED;
    
    for(const auto &[name, engine] : _engines) {
        ret_code r_action = RC_UNKNOWN;
        
        switch (i_cmd) {
            case ENGINES_EVENT_REGISTER:       { r_action = engine->subscribe(i_args.event, i_args.observer);   } break;
            case ENGINES_EVENT_UNREGISTER:     { r_action = engine->unsubscribe(i_args.event, i_args.observer); } break;
            case ENGINES_EVENT_UNREGISTER_ALL: { r_action = engine->remove_the_observer(i_args.observer);       } break;
            default: break;
        }
        
        if(RC_OK == r_action) {
            LOG_INFO("[Driver]\t[YES] 订阅者：%s 事件：%d 执行动作：%s -> 受理引擎：%s；错误码：%d",
                     i_args.observer->get_name().c_str(),
                     i_args.event,
                     cmd2string(i_cmd),
                     name.c_str(),
                     r_action
            );
            
            if(ENGINES_EVENT_UNREGISTER_ALL != i_cmd) {
                r_code = r_action; break; // 目前，除全注销指令外的事件类型指令，一个事件只应该给一个事件引擎受理。
            }
        } else {
            o_names.insert(name);
        }
    }
    
    return (ENGINES_EVENT_UNREGISTER_ALL == i_cmd) ? RC_OK : r_code;
}

const char *EngineManager::cmd2string(ControlCommand i_cmd)
{
    static const std::string unknown{"unknown"};
    
    const auto it = _cmd2string.find(i_cmd);
    
    return (_cmd2string.end() != it) ? it->second.c_str() : unknown.c_str();
}
