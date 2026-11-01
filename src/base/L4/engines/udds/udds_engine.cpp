
#include <thread>
#include <strings.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include "header/consts.hpp"
#include "header/schema/schema.hpp"
#include "engines/udds/udds_engine.hpp"

UDDSEngine::UDDSEngine()
{
    _fd_socket = -1;
}

UDDSEngine::~UDDSEngine()
{
    if(_fd_socket > 0) {
        close(_fd_socket); _fd_socket = -1;
    }
}

ret_code UDDSEngine::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(RC_OK != (r_code = socket_create())) {
            break;
        }

    } while (false);

    return r_code;
}

ret_code UDDSEngine::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        this->_is_exit = false;

        std::thread thread_for_read([this](){
            this->socket_read_worker_thread();
        }); thread_for_read.detach();

        r_code = RC_OK;
    } while (false);

    return r_code;
}

ret_code UDDSEngine::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        this->_is_exit = true;

        r_code = RC_OK;
    } while(false);

    return r_code;
}

ret_code UDDSEngine::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        this->_is_exit = true;

        if(_fd_socket > 0) {
            close(_fd_socket); _fd_socket = -1;
        }

        r_code = RC_OK;
    } while(false);

    return r_code;
}

ret_code UDDSEngine::subscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(EVENT_TYPE_GUDS != i_event) {
            r_code = RC_UNCONCERNED; break;
        }

        r_code = _observer_mgr.observer_add(i_observer);
    } while (false);

    return r_code;
}

ret_code UDDSEngine::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(EVENT_TYPE_GUDS != i_event) {
            r_code = RC_UNCONCERNED; break;
        }

        r_code = _observer_mgr.observer_del(i_observer);
    } while (false);

    return r_code;
}

ret_code UDDSEngine::remove_the_event(EventType i_event)
{
    return RC_UNCONCERNED;
}

ret_code UDDSEngine::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_ARG_ERR; break; }

        r_code = _observer_mgr.observer_del(i_observer);
    } while (false);

    return r_code;
}

ret_code UDDSEngine::socket_create()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        _fd_socket = ::socket(AF_UNIX, SOCK_DGRAM, 0);
        if(0 > _fd_socket) {
            r_code = RC_SOCKET_INIT_ERR; break;
        }

        struct timeval tv{ .tv_sec = 2, .tv_usec = 0 };

        ::setsockopt(_fd_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
//        ::setsockopt(_fd_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        struct sockaddr_un addr;
        addr.sun_family  = AF_UNIX;
        // static_assert(sizeof(SOCK_PATH_UDDS_DATA) < sizeof(addr.sun_path));
        // ::strncpy(addr.sun_path, SOCK_PATH_UDDS_DATA, sizeof(addr.sun_path) -1);

        ::unlink(addr.sun_path);

        int rc_bind = ::bind(_fd_socket.load(), (struct sockaddr *)&addr, sizeof(addr));
        if(0 != rc_bind) {
            r_code = RC_SOCKET_BIND_ERR; break;
        }

        ::chmod(addr.sun_path, 0666);

        r_code = RC_OK;
    } while (false);

    return r_code;
}

template <typename T, typename... Args>
std::shared_ptr<T[]> make_shared_array_nothrow(size_t size, Args&&... args)
{
    T* raw_ptr = new (std::nothrow) T[size]{std::forward<Args>(args)...};
    if (!raw_ptr) {
        return nullptr;
    }

    return std::shared_ptr<T[]>(raw_ptr, std::default_delete<T[]>());
}

void UDDSEngine::socket_read_worker_thread()
{
    struct sockaddr_un addr;
    addr.sun_family  = AF_UNIX;
    socklen_t addr_len = sizeof(addr);

    while(!this->_is_exit) {
        // 这里需要注意结构体是变长的，需要额外申请一个sizeof(SCHEMA)
        std::shared_ptr<char[]> buff = make_shared_array_nothrow<char>(offsetof(Event, body.udds.msg) + sizeof(SCHEMA));
        if (nullptr == buff) {
            ::sleep(1); continue;
        }

        std::shared_ptr<Event> msgPtr(buff, (Event*)buff.get());

        msgPtr->head.event_type     = EVENT_TYPE_GUDS;
        msgPtr->head.event_producer = EP_GUDS;
        msgPtr->head.event_body_len = offsetof(EventBodyUDDS, msg) + sizeof(SCHEMA);

        ssize_t io_size = ::recvfrom(_fd_socket, msgPtr->body.udds.msg, sizeof(SCHEMA), MSG_NOSIGNAL, (struct sockaddr*)&addr, &addr_len);
        if (0 >= io_size) {
            int err = errno;
            if (EWOULDBLOCK == err || EAGAIN == err || EINTR == err) {
                continue;
            }
            ::sleep(1); continue;
        }

        // 如果客户端没有bind，则 0 == addr_len，即不能通过sento回复客户端
        // 否则 0 < addr_len，随后可使用addr回复客户端

        msgPtr->body.udds.msg_len = io_size;

        _observer_mgr.observer_notify(msgPtr);
    }
}