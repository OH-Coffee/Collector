
#ifndef netlink_connector_event_handler_hpp
#define netlink_connector_event_handler_hpp

#include <list>
#include <memory>
#include <mutex>

#include <strings.h>
#include <sys/fanotify.h>

#include "header/errcode.hpp"
#include "process/proc_info.hpp"

#include "engines/event_observer.hpp"
#include "engines/event_observer_manager.hpp"
#include "engines/netlink/connector/connector_message.hpp"

class ConnectorEventHandler : public EventObserverManager
{
public:
             ConnectorEventHandler() = default;
    virtual ~ConnectorEventHandler() = default;
public:
    virtual ConnectorProcEventType  event()                                          = 0;
    virtual ret_code                event_handler(const netlink_connector_message *) = 0;
};

typedef std::shared_ptr<ConnectorEventHandler> ConnectorEventHandlerPtr;

#endif // netlink_connector_event_handler_hpp
