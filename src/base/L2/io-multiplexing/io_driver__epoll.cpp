
#include <cerrno>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "io-multiplexing/io_driver__epoll.hpp"

IO_DriverEpoll::IO_DriverEpoll() : IO_Driver()
{
    _epoll_fd   = -1;
    _is_exit    = true;
    _timeout_ms = 1000;
}

IO_DriverEpoll::~IO_DriverEpoll()
{
    this->close();
}

ret_code IO_DriverEpoll::init()
{
    ret_code r_code = RC_UNKNOWN;
    do {
        this->create();
        if(_epoll_fd < 0) { r_code = RC_IO_DRIVER_EPOLL_CREATE_ERR; break; }

        _is_exit = false;
        r_code = RC_OK;
    } while (false);
    return r_code;
}

ret_code IO_DriverEpoll::start()
{
    ret_code r_code = RC_UNKNOWN;

    struct epoll_event epoll_events[32];
    while(!_is_exit) {
        int event_num = epoll_wait(_epoll_fd, epoll_events, 32, _timeout_ms);
        if(event_num < 0) {
            if(errno == EINTR) {
                continue;
            } else {
                r_code = RC_IO_DRIVER_EPOLL_WAIT_ERR; break;
            }
        }

        for(int i = 0; i < event_num; i++) {
            int fd      = epoll_events[i].data.fd;
            int events  = IO_EVENT_UNKNOWN;

            if(EPOLLIN  == (epoll_events[i].events & EPOLLIN))  { events |= IO_EVENT_READ;  }
            if(EPOLLOUT == (epoll_events[i].events & EPOLLOUT)) { events |= IO_EVENT_WRITE; }

            IO_Handler *handler = nullptr;
            {
                std::lock_guard<std::mutex> mutex_guard(_mutex_for_io_handlers);
                auto it = _io_handlers.find(fd);
                handler = (it != _io_handlers.end()) ? it->second : nullptr;
            }

            if(nullptr != handler) { try {
                handler->io_handler(fd, events);
            } catch (...) {} }
        }
    }

    return r_code;
}

ret_code IO_DriverEpoll::stop()
{
    this->_is_exit = true; return RC_OK;
}

ret_code IO_DriverEpoll::uninit()
{
    ret_code r_code = RC_UNKNOWN;
    do {
        r_code = this->stop(); this->close();
    } while (false);
    return r_code;
}

void IO_DriverEpoll::create()
{
    if(_epoll_fd < 0) { _epoll_fd = epoll_create1(EPOLL_CLOEXEC); }
}
void IO_DriverEpoll::close()
{
    if(_epoll_fd > 0) { ::close(_epoll_fd); _epoll_fd = -1; }
}

ret_code IO_DriverEpoll::insert(int i_fd, IO_Handler *i_io_handler)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_fd < 0)                { r_code = RC_IO_DRIVER_INVALID_FD; break; }
        if(nullptr == i_io_handler) { r_code = RC_ARG_ERR;              break; }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_io_handlers);

            if(_io_handlers.end() != _io_handlers.find(i_fd)) {
                r_code = RC_IO_DRIVER_REPEATED_FD; break;
            } else {
                struct epoll_event ev = {}; bzero(&ev, sizeof(ev));

                ev.data.fd  = i_fd;
                if(IO_EVENT_READ  == (i_io_handler->io_events() & IO_EVENT_READ))  { ev.events |= EPOLLIN;  }
                if(IO_EVENT_WRITE == (i_io_handler->io_events() & IO_EVENT_WRITE)) { ev.events |= EPOLLOUT; }

                if(0 != epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, i_fd, &ev)) {
                    r_code = RC_IO_DRIVER_EPOLL_CTL_ERR; break;
                } else {
                    _io_handlers[i_fd] = i_io_handler; r_code = RC_OK;
                }
            }
        }
    } while (false);

    return r_code;
}

ret_code IO_DriverEpoll::erase(int i_fd, IO_Handler *i_io_handler)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_fd < 0) { r_code = RC_IO_DRIVER_INVALID_FD; break; }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_io_handlers);

            if(_io_handlers.end() == _io_handlers.find(i_fd)) {
                r_code = RC_IO_DRIVER_INVALID_FD; break;
            } else {
                if(0 != epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, i_fd, nullptr)) {
                    r_code = RC_IO_DRIVER_EPOLL_CTL_ERR; break;
                } else {
                    r_code = RC_OK; _io_handlers.erase(i_fd);
                }
            }
        }
    } while (false);

    return r_code;
}