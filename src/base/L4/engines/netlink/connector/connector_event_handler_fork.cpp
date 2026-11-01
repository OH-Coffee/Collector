
#include "engines/netlink/connector/connector_event_handler_fork.hpp"

ret_code ConnectorEventHandlerFork::event_handler(const netlink_connector_message *i_data)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr  == i_data) { break; }

        auto msg  = NEW_EVENT_MESSAGE();
        auto head = &msg->head;
        auto body = &msg->body.clone;
        const static int64_t body_len = sizeof(*body);

        head->event_type        = EVENT_TYPE_CLONE;
        head->event_producer    = EP_NETLINK_CONNECTOR;
        head->event_process.pid = i_data->nl_body.data.proc_event.event_data.fork.parent_pid;
        head->event_process.uid = -1;
        head->event_body_len    = body_len;

        body->pid_parent    = i_data->nl_body.data.proc_event.event_data.fork.parent_pid;
        body->pid_child     = i_data->nl_body.data.proc_event.event_data.fork.child_pid;

        r_code = ConnectorEventHandler::observer_notify(msg);
    } while (false);

    return r_code;
}
