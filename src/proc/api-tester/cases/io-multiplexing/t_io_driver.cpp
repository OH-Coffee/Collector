#include <chrono>
#include <future>
#include <thread>

#include <unistd.h>

#include "gtest/gtest.h"
#include "io-multiplexing/io_driver_factory.hpp"

namespace {

class PipePair
{
public:
    bool open()
    {
        return 0 == pipe(_fds);
    }

    ~PipePair()
    {
        if(_fds[0] >= 0) { close(_fds[0]); }
        if(_fds[1] >= 0) { close(_fds[1]); }
    }

    int read_fd() const { return _fds[0]; }
    int write_fd() const { return _fds[1]; }

private:
    int _fds[2]{-1, -1};
};

class ReadablePipeHandler : public IO_Handler
{
public:
    explicit ReadablePipeHandler(std::promise<int>& i_events) : _events(i_events) {}

    int io_events() override
    {
        return IO_EVENT_READ;
    }

    int io_handler(int i_fd, int i_events) override
    {
        char byte = '\0';
        const ssize_t bytes_read = read(i_fd, &byte, sizeof(byte));
        _events.set_value(bytes_read == sizeof(byte) ? i_events : IO_EVENT_UNKNOWN);
        return 0;
    }

private:
    std::promise<int>& _events;
};

TEST(IODriverFactoryTest, CreatesSupportedDriverAndRejectsUnknownType)
{
    EXPECT_NE(IO_DriverFactory::new_driver(IO_DRIVER_EPOLL), nullptr);
    EXPECT_EQ(IO_DriverFactory::new_driver(IO_DRIVER_UNKNOWN), nullptr);
}

TEST(IODriverEpollTest, ValidatesDescriptorsAndHandlerPointers)
{
    IO_DriverPtr driver = IO_DriverFactory::new_driver(IO_DRIVER_EPOLL);
    ASSERT_NE(driver, nullptr);
    ASSERT_EQ(driver->init(), RC_OK);

    EXPECT_EQ(driver->insert(-1, nullptr), RC_IO_DRIVER_INVALID_FD);
    EXPECT_EQ(driver->insert(0, nullptr), RC_ARG_ERR);
    EXPECT_EQ(driver->erase(-1, nullptr), RC_IO_DRIVER_INVALID_FD);
    EXPECT_EQ(driver->uninit(), RC_OK);
}

TEST(IODriverEpollTest, DispatchesReadableDescriptorAndRemovesRegistration)
{
    PipePair pipe_pair;
    ASSERT_TRUE(pipe_pair.open());

    IO_DriverPtr driver = IO_DriverFactory::new_driver(IO_DRIVER_EPOLL);
    ASSERT_NE(driver, nullptr);
    ASSERT_EQ(driver->init(), RC_OK);

    std::promise<int> received_events;
    std::future<int> event_result = received_events.get_future();
    ReadablePipeHandler handler(received_events);

    ASSERT_EQ(driver->insert(pipe_pair.read_fd(), &handler), RC_OK);
    EXPECT_EQ(driver->insert(pipe_pair.read_fd(), &handler), RC_IO_DRIVER_REPEATED_FD);

    const char byte = 'x';
    ASSERT_EQ(write(pipe_pair.write_fd(), &byte, sizeof(byte)), 1);
    std::thread event_loop([&driver] {
        driver->start();
    });
    const std::future_status event_status = event_result.wait_for(std::chrono::seconds(2));

    if(event_status == std::future_status::ready) {
        EXPECT_EQ(event_result.get(), IO_EVENT_READ);
        EXPECT_EQ(driver->erase(pipe_pair.read_fd(), &handler), RC_OK);
        EXPECT_EQ(driver->erase(pipe_pair.read_fd(), &handler), RC_IO_DRIVER_INVALID_FD);
    }
    EXPECT_EQ(driver->stop(), RC_OK);
    event_loop.join();
    EXPECT_EQ(event_status, std::future_status::ready);
    EXPECT_EQ(driver->uninit(), RC_OK);
}

} // namespace
