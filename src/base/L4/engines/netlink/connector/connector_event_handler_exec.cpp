
#include "engines/netlink/connector/connector_event_handler_exec.hpp"


ret_code ConnectorEventHandlerExec::event_handler(const netlink_connector_message *i_data)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr  == i_data) { break; }

        auto msg  = NEW_EVENT_MESSAGE();
        auto head = &msg->head;
        auto body = &msg->body.exec;
        const static int64_t body_len = sizeof(*body);

        head->event_type        = EVENT_TYPE_EXEC;
        head->event_producer    = EP_NETLINK_CONNECTOR;
        head->event_process.pid = i_data->nl_body.data.proc_event.event_data.exec.process_pid;
        head->event_process.uid = -1;
        head->event_body_len    = body_len;

        bzero(body->path, BUF_LEN_EXEC_PATH);
        bzero(body->argv, BUF_LEN_EXEC_ARGV);

        r_code = ConnectorEventHandler::observer_notify(msg);
    } while (false);

    return r_code;
}
