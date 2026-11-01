
#include <thread>

#include <strings.h>
#include <sys/socket.h>

#include "engines/netlink/connector/connector_engine.hpp"
#include "engines/netlink/connector/connector_message.hpp"
#include "engines/netlink/connector/connector_event_handler_z_factory.hpp"

ConnectorEngine::ConnectorEngine()
{
    _fd_socket = -1;
}

ConnectorEngine::~ConnectorEngine()
{
    if(_fd_socket > 0) {
        close(_fd_socket); _fd_socket = -1;
    }
}

ret_code ConnectorEngine::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(RC_OK != (r_code = netlink_connector_create())) {
            break;
        }

        // 创建IO驱动器，为事件源提供驱动能力。
        _io_driver = std::make_shared<IO_DriverEpoll>();
        if(nullptr == _io_driver) {
            r_code = RC_IO_DRIVER_CREATE_ERR; break;
        }

        // IO驱动器初始化
        if(RC_OK != (r_code = _io_driver->init()))                      { break; }
        if(RC_OK != (r_code = _io_driver->insert(_fd_socket, this)))    { break; }
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 创建线程，IO驱动器开始工作
        std::thread thread_io_driver([this](){
            this->_io_driver->start();
        }); thread_io_driver.detach();

        r_code = netlink_connector_listen(true);
        if(r_code != RC_OK) { break; }
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->_io_driver->stop();
        r_code = netlink_connector_listen(false);
    } while(false);

    return r_code;
}

ret_code ConnectorEngine::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->_io_driver->uninit();

        if(_fd_socket > 0) {
            close(_fd_socket); _fd_socket = -1;
        }
    } while(false);

    return r_code;
}

ret_code ConnectorEngine::subscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        auto event_handler = ConnectorEventHandlerFactory::create(i_event);
        if(nullptr == event_handler) {
            r_code = RC_UNCONCERNED; break;
        }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            auto it = _event_handlers.find(event_handler->event());
            if(_event_handlers.end() != it) {
                event_handler = it->second;
            } else {
                _event_handlers[event_handler->event()] = event_handler;
            }
        }
        if(nullptr == event_handler) { r_code = RC_FAIL; break; }

        r_code = event_handler->observer_add(i_observer);
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        if(nullptr == ConnectorEventHandlerFactory::create(i_event)) {
            r_code = RC_UNCONCERNED; break;
        }

        ConnectorEventHandlerPtr event_handler = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            auto it = _event_handlers.find(event_handler->event());
            if(_event_handlers.end() != it) {
                event_handler = it->second;
            }
        }
        if(nullptr == event_handler) { r_code = RC_UNCONCERNED; break; }

        r_code = event_handler->observer_del(i_observer);
        if(event_handler->observer_num() <= 0) {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);
            _event_handlers.erase(event_handler->event());
        }
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::remove_the_event(EventType i_event)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        auto event_handler = ConnectorEventHandlerFactory::create(i_event);
        if(nullptr == event_handler) {
            r_code = RC_UNCONCERNED; break;
        }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);
            if(_event_handlers.erase(event_handler->event()) > 0) {
                r_code = RC_OK;
            }
        }
    } while (false);

    return r_code;
}
ret_code ConnectorEngine::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_ARG_ERR; break; }

        ConnectorEventHandlerPtr event_handler = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            for(auto it = _event_handlers.begin(); it != _event_handlers.end(); ) {
                if(nullptr == (event_handler = it->second)) { ++it; continue; }

                if(RC_OK == event_handler->observer_del(i_observer)) {
                    r_code = RC_OK;
                }

                it = (event_handler->observer_num() <= 0) ? _event_handlers.erase(it) : ++it;
            }
        }
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::netlink_connector_create()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _fd_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
        if(_fd_socket < 0) {
            r_code = RC_SOCKET_INIT_ERR; break;
        }

        struct sockaddr_nl netlink_addr = {};
        netlink_addr.nl_family  = AF_NETLINK;
        netlink_addr.nl_groups  = CN_IDX_PROC;
        netlink_addr.nl_pid     = getpid();

        int rc_bind = bind(_fd_socket.load(), (struct sockaddr *)&netlink_addr, sizeof(netlink_addr));
        if(0 == rc_bind) {
            r_code = RC_OK;
        } else {
            r_code = RC_SOCKET_BIND_ERR; break;
        }
    } while (false);

    return r_code;
}

ret_code ConnectorEngine::netlink_connector_listen(bool i_listen)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        netlink_connector_message   netlink_connector_msg;
        uint64_t                    netlink_connector_msg_len = sizeof(netlink_connector_msg);
        bzero(&netlink_connector_msg, netlink_connector_msg_len);

        netlink_connector_msg.nl_head.nlmsg_len     = netlink_connector_msg_len;
        netlink_connector_msg.nl_head.nlmsg_pid     = getpid();
        netlink_connector_msg.nl_head.nlmsg_type    = NLMSG_DONE;
        netlink_connector_msg.nl_head.nlmsg_flags   = 0;
        netlink_connector_msg.nl_head.nlmsg_seq     = 0;

        netlink_connector_msg.nl_body.cn_msg.id.idx = CN_IDX_PROC;
        netlink_connector_msg.nl_body.cn_msg.id.val = CN_VAL_PROC;
        netlink_connector_msg.nl_body.cn_msg.seq    = 0;
        netlink_connector_msg.nl_body.cn_msg.ack    = 0;
        netlink_connector_msg.nl_body.cn_msg.len    = sizeof(enum proc_cn_mcast_op);
        netlink_connector_msg.nl_body.cn_msg.flags  = 0;

        netlink_connector_msg.nl_body.data.cn_mcast = i_listen ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

        ssize_t rc_send = send(_fd_socket, &netlink_connector_msg, netlink_connector_msg_len, 0);
        r_code = (rc_send > 0) ? RC_OK : RC_SOCKET_SEND_ERR; break;
    } while (false);

    return r_code;
}
