
#ifndef fanotify_handler_hpp
#define fanotify_handler_hpp

#include <list>
#include <memory>
#include <mutex>

#include <strings.h>
#include <sys/fanotify.h>

#include "header/errcode.hpp"
#include "process/proc_info.hpp"
#include "engines/event_observer.hpp"
#include "engines/event_observer_manager.hpp"

typedef uint64_t FanotifyEventType;


class FanotifyEventHandler : public EventObserverManager
{
public:
             FanotifyEventHandler() = default;
    virtual ~FanotifyEventHandler() = default;
public:
    virtual FanotifyEventType   event()                                               = 0;
    virtual ret_code            event_handler(const struct fanotify_event_metadata *) = 0;
};

typedef std::shared_ptr<FanotifyEventHandler> FanotifyEventHandlerPtr;

#endif // fanotify_handler_hpp
