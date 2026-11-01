
#ifndef io_driver_epoll_hpp
#define io_driver_epoll_hpp

#include <atomic>
#include <mutex>

#include "io-multiplexing/io_driver.hpp"

class IO_DriverEpoll : public IO_Driver
{
public:
     IO_DriverEpoll();
    ~IO_DriverEpoll() override;
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    ret_code insert(int, IO_Handler *)   override;
    ret_code erase(int,  IO_Handler *)   override;
private:
    void create();
    void close();
private:
    int                         _epoll_fd{-1};
private:
    std::atomic_bool            _is_exit{true};
    std::atomic_int32_t         _timeout_ms{1000};
private:
    std::map<int, IO_Handler *> _io_handlers;
    std::mutex                  _mutex_for_io_handlers;
};

#endif  // io_driver_epoll_hpp