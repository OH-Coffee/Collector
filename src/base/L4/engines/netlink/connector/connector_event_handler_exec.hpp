
#ifndef netlink_connector_event_handler_exec_hpp
#define netlink_connector_event_handler_exec_hpp

#include "engines/netlink/connector/connector_event_handler.hpp"

class ConnectorEventHandlerExec : public ConnectorEventHandler
{
public:
    ConnectorProcEventType event() override { return proc_cn_event::PROC_EVENT_EXEC; }
public:
    ret_code event_handler(const netlink_connector_message *) override;
};

#endif // netlink_connector_event_handler_exec_hpp
