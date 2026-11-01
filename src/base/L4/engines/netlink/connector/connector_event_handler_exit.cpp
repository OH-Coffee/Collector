
#include "engines/netlink/connector/connector_event_handler_exit.hpp"

ret_code ConnectorEventHandlerExit::event_handler(const netlink_connector_message *i_data)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr  == i_data) { break; }

        auto msg  = NEW_EVENT_MESSAGE();
        auto head = &msg->head;
        auto body = &msg->body.exit_group;
        const static int64_t body_len = sizeof(*body);

        head->event_type        = EVENT_TYPE_EXIT_GROUP;
        head->event_producer    = EP_NETLINK_CONNECTOR;
        head->event_process.pid = i_data->nl_body.data.proc_event.event_data.exit.process_pid;
        head->event_process.uid = -1;
        head->event_body_len    = body_len;

        body->exit_code     = i_data->nl_body.data.proc_event.event_data.exit.exit_code;
        body->exit_signal   = i_data->nl_body.data.proc_event.event_data.exit.exit_signal;

        r_code = ConnectorEventHandler::observer_notify(msg);
    } while (false);

    return r_code;
}
