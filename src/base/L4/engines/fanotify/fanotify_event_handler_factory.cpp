
#include "engines/fanotify/fanotify_event_handler_factory.hpp"

#include "engines/fanotify/fanotify_event_handler__close_read.hpp"
#include "engines/fanotify/fanotify_event_handler__close_write.hpp"

FanotifyEventHandlerPtr FanotifyEventHandlerFactory::create(EventType i_type)
{
    FanotifyEventHandlerPtr r_handler = nullptr;

    switch (i_type) {
        case EVENT_TYPE_RCLOSE: {
            r_handler = std::make_shared<FanotifyEventHandlerCloseRead>();
        } break;
        case EVENT_TYPE_WCLOSE: {
            r_handler = std::make_shared<FanotifyEventHandlerCloseWrite>();
        } break;
        default: { r_handler = nullptr; } break;
    }

    return r_handler;
}
