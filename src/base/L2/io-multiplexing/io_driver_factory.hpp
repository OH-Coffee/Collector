
#ifndef io_driver_z_factory_hpp
#define io_driver_z_factory_hpp

#include "io-multiplexing/io_driver.hpp"

typedef enum
{
    IO_DRIVER_UNKNOWN = 0,
    IO_DRIVER_EPOLL,
} IO_DriverType;

class IO_DriverFactory
{
public:
    static IO_DriverPtr new_driver(IO_DriverType);
};

#endif // io_driver_z_factory_hpp