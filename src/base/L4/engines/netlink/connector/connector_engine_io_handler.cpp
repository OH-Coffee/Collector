
#include "engines/netlink/connector/connector_engine.hpp"
#include "engines/netlink/connector/connector_message.hpp"

int ConnectorEngine::io_events()
{
    return IO_EVENT_READ;   // 用于告知IO-Driver，仅关注可读事件
}

int ConnectorEngine::io_handler(int i_fd, int i_events)
{
    int r_code = 0;

    do {
        if(i_fd != _fd_socket) { break; }

        struct sockaddr_nl          netlink_addr;
        socklen_t                   netlink_addr_len;

        netlink_connector_message   netlink_connector_msg;
        uint64_t                    netlink_connector_msg_len = sizeof(netlink_connector_msg);

        ssize_t rc_recv = recvfrom(_fd_socket,
                                   &netlink_connector_msg,
                                   netlink_connector_msg_len,
                                   MSG_DONTWAIT | MSG_NOSIGNAL,
                                   (struct sockaddr *)&netlink_addr,
                                   &netlink_addr_len
        );
        if(0 == rc_recv) {

        } else if(-1 == rc_recv) {
            switch (errno) {

            }
        } else if(!_pause.load(std::memory_order_relaxed)) {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            const auto it = _event_handlers.find(netlink_connector_msg.nl_body.data.proc_event.what);
            if(_event_handlers.end() != it) {
                it->second->event_handler(&netlink_connector_msg);
            }
        }
    } while (false);

    return r_code;
}
