
#ifndef fanotify_handler_factory_hpp
#define fanotify_handler_factory_hpp

#include "engines/fanotify/fanotify_event_handler.hpp"

class FanotifyEventHandlerFactory
{
public:
    static FanotifyEventHandlerPtr create(EventType);
};

#endif // fanotify_handler_factory_hpp
