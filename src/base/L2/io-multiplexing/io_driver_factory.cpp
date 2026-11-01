
#include "io-multiplexing/io_driver__epoll.hpp"
#include "io-multiplexing/io_driver_factory.hpp"

IO_DriverPtr IO_DriverFactory::new_driver(IO_DriverType i_type)
{
    IO_DriverPtr r_ptr = nullptr;

    switch (i_type) {
        case IO_DRIVER_EPOLL: { r_ptr = std::make_shared<IO_DriverEpoll>(); } break;
        default: { r_ptr = nullptr; } break;
    }

    return r_ptr;
}