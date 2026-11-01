
#include "engines/netlink/connector/connector_event_handler_exec.hpp"
#include "engines/netlink/connector/connector_event_handler_exit.hpp"
#include "engines/netlink/connector/connector_event_handler_fork.hpp"
#include "engines/netlink/connector/connector_event_handler_z_factory.hpp"

ConnectorEventHandlerPtr ConnectorEventHandlerFactory::create(EventType i_type)
{
    ConnectorEventHandlerPtr r_handler = nullptr;

    switch (i_type) {
        case EVENT_TYPE_CLONE:      { r_handler = std::make_shared<ConnectorEventHandlerFork>(); } break;
        case EVENT_TYPE_EXEC:       { r_handler = std::make_shared<ConnectorEventHandlerExec>(); } break;
        case EVENT_TYPE_EXIT_GROUP: { r_handler = std::make_shared<ConnectorEventHandlerExit>(); } break;

        default: { r_handler = nullptr; } break;
    }

    return r_handler;
}
