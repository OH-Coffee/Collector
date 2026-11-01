
#ifndef fanotify_handler_close_write_hpp
#define fanotify_handler_close_write_hpp

#include "engines/fanotify/fanotify_event_handler.hpp"

class FanotifyEventHandlerCloseWrite : public FanotifyEventHandler
{
public:
    FanotifyEventType event() override { return FAN_CLOSE_WRITE; }
public:
    ret_code event_handler(const struct fanotify_event_metadata *) override;
};

ret_code FanotifyEventHandlerCloseWrite::event_handler(const struct fanotify_event_metadata *i_data)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr  == i_data)      { break; }
        if(FAN_NOFD == i_data->fd)  { break; }

        static const int64_t body_len = sizeof(EventBodyWClose);
        static const int64_t path_len = sizeof(EventBodyWClose::path);

        auto msg  = NEW_EVENT_MESSAGE();
        auto head = &msg->head;
        auto body = &msg->body.wclose;

        head->event_type        = EVENT_TYPE_WCLOSE;
        head->event_producer    = EP_FANOTIFY;
        head->event_process.pid = i_data->pid;
        head->event_process.uid = -1;
        head->event_body_len    = body_len;

        bzero(body->path, body_len);
        proc_readlink("/proc/self/fd/" + std::to_string(i_data->fd), path_len, body->path);

        r_code = FanotifyEventHandler::observer_notify(msg);
    } while (false);

    return r_code;
}

#endif // fanotify_handler_close_write_hpp
