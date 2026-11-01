
#include "engines/event_engine_factory.hpp"

#if defined(OS_MACOS)
    #include "engines/es/endpoint_security_engine.hpp"
#elif defined(OS_LINUX)
    #include "engines/fanotify/fanotify_engine.hpp"
    #include "engines/bpf/tracepoint/tracepoint_engine.hpp"
    #include "engines/bpf/kprobe/kprobe_engine.hpp"
    #include "engines/netlink/connector/connector_engine.hpp"
    #include "engines/udds/udds_engine.hpp"
#endif

EventEnginePtr EventEngineFactory::create(const std::string &i_name)
{
    EventEnginePtr r_engine = nullptr;

    do {
        if(i_name.empty()) { break; }
        
#if defined(OS_MACOS)
        if(i_name == ENGINE_NAME_ES)          { r_engine = NEW_ENDPOINT_SECURITY_ENGINE();    break; }
#elif defined(OS_LINUX)
        if(i_name == ENGINE_NAME_FANOTIFY)    { r_engine = NEW_FANOTIFY_ENGINE();             break; }
        if(i_name == ENGINE_NAME_TRACEPOINT)  { r_engine = NEW_TRACEPOINT_ENGINE();           break; }
        if(i_name == ENGINE_NAME_KPROBE)      { r_engine = NEW_KPROBE_ENGINE();               break; }
        if(i_name == ENGINE_NAME_CONNECTOR)   { r_engine = NEW_CONNECTOR_ENGINE();            break; }
        if(i_name == ENGINE_NAME_UDDS)        { r_engine = NEW_UDDS_ENGINE();                 break; }
#endif
    } while (false);

    return r_engine;
}
