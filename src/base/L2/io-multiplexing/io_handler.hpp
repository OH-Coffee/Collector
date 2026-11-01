
#ifndef io_handler_hpp
#define io_handler_hpp

typedef enum : int
{
    IO_EVENT_UNKNOWN    = (int)0,
    IO_EVENT_READ       = (int)(0x01 << 0),
    IO_EVENT_WRITE      = (int)(0x01 << 1),
    IO_EVENT_ALL        = (int)(IO_EVENT_READ | IO_EVENT_WRITE),
} IO_EVENT;

class IO_Handler
{
public:
             IO_Handler() = default;
    virtual ~IO_Handler() = default;
public:
    virtual int io_events()                        = 0;
    virtual int io_handler(int i_fd, int i_events) = 0;
};

#endif // io_handler_hpp