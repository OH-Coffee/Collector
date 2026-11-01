
#ifndef event_engines_bpf_tracepoint_engine_hpp
#define event_engines_bpf_tracepoint_engine_hpp

#include "header/errcode.hpp"
#include "engines/event_observer.hpp"


#define ENGINE_NAME_KPROBE            "collector.engine.kprobe"
#define ENGINE_NAME_RAWSYSCALL        "collector.engine.rawsyscall"
#define ENGINE_NAME_TRACEPOINT        "collector.engine.tracepoint"
#define ENGINE_NAME_ES                "collector.engine.es"
#define ENGINE_NAME_FANOTIFY          "collector.engine.fanotify"
#define ENGINE_NAME_INOTIFY           "collector.engine.inotify"
#define ENGINE_NAME_CONNECTOR         "collector.engine.connector"
#define ENGINE_NAME_UDDS              "collector.engine.udds"

typedef struct EventEngineArgs
{
    EventType           event_type;
    EventObserver *     event_observer;

    uint64_t            extend_data_len;
    uint8_t             extend_data[0];
} EventEngineArgs;


class EventEngine
{
public:
             EventEngine() = default;
    virtual ~EventEngine() = default;
public:
    virtual ret_code    init()                                      = 0;
    virtual ret_code    start()                                     = 0;
    virtual ret_code    stop()                                      = 0;
    virtual ret_code    uninit()                                    = 0;
public:
    virtual void        pause()                                     = 0;
    virtual void        resume()                                    = 0;
public:
    virtual ret_code      subscribe(EventType, EventObserver *)     = 0;
    virtual ret_code    unsubscribe(EventType, EventObserver *)     = 0;
public:
    virtual ret_code    remove_the_event(EventType)                 = 0;
    virtual ret_code    remove_the_observer(EventObserver *)        = 0;
public:
    virtual std::string name()                                      = 0;
};

typedef std::shared_ptr<EventEngine> EventEnginePtr;

#endif // event_engines_bpf_tracepoint_engine_hpp
