#ifndef netlink_connector_event_handler_fork_hpp
#define netlink_connector_event_handler_fork_hpp

#include "engines/netlink/connector/connector_event_handler.hpp"

class ConnectorEventHandlerFork : public ConnectorEventHandler
{
public:
    ConnectorProcEventType event() override { return proc_cn_event::PROC_EVENT_FORK; }
public:
    ret_code event_handler(const netlink_connector_message *) override;
};

#endif // netlink_connector_event_handler_fork_hpp
