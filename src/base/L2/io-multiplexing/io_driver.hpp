
#ifndef io_driver_hpp
#define io_driver_hpp

#include <map>
#include <memory>

#include "header/errcode.hpp"
#include "io-multiplexing/io_handler.hpp"

class IO_Driver
{
public:
             IO_Driver() = default;
    virtual ~IO_Driver() = default;
public:
    virtual ret_code init()     = 0;
    virtual ret_code start()    = 0;
    virtual ret_code stop()     = 0;
    virtual ret_code uninit()   = 0;
public:
    virtual ret_code insert(int, IO_Handler *)   = 0;
    virtual ret_code erase(int,  IO_Handler *)   = 0;
};
typedef std::shared_ptr<IO_Driver> IO_DriverPtr;

#endif // io_driver_hpp