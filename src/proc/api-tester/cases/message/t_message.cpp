#include <chrono>
#include <future>
#include <mutex>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "message/message_client.hpp"
#include "message/message_server.hpp"

namespace {

using namespace std::chrono_literals;

class RecordingClient : public message::MessageClient
{
public:
    RecordingClient(const std::string& i_name, message::MessageServer* i_server,
                    std::vector<std::string>* i_events = nullptr,
                    std::mutex* i_events_mutex = nullptr)
        : message::MessageClient(i_name, i_server),
          _events(i_events),
          _events_mutex(i_events_mutex)
    {
    }

    ret_code message_handler(uint64_t i_type, uint8_t* i_buf, size_t i_len) override
    {
        types.push_back(i_type);
        if(i_len > 0) {
            payloads.emplace_back(i_buf, i_buf + i_len);
        } else {
            payloads.emplace_back();
        }
        if(nullptr != _events && nullptr != _events_mutex) {
            std::lock_guard<std::mutex> lock(*_events_mutex);
            _events->emplace_back("handler");
        }
        return handler_result;
    }

    std::vector<uint64_t> types;
    std::vector<std::vector<uint8_t>> payloads;
    ret_code handler_result{RC_OK};

private:
    std::vector<std::string>* _events;
    std::mutex* _events_mutex;
};

class RecordingObserver : public message::MessageObserver
{
public:
    RecordingObserver(std::vector<std::string>& i_events, std::mutex& i_mutex,
                      std::promise<void>& i_finished)
        : _events(i_events), _mutex(i_mutex), _finished(i_finished)
    {
    }

    ret_code on_send_begin(message::MESSAGE_PTR) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _events.emplace_back("begin");
        return RC_OK;
    }

    ret_code on_send_finish(message::MESSAGE_PTR) override
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _events.emplace_back("finish");
        }
        _finished.set_value();
        return RC_OK;
    }

private:
    std::vector<std::string>& _events;
    std::mutex& _mutex;
    std::promise<void>& _finished;
};

TEST(MessageTest, CopiesPayloadAndSupportsEmptyMessages)
{
    uint8_t payload[] = {1, 2, 3};
    message::Message copied_message(42, payload, sizeof(payload));

    payload[0] = 9;
    EXPECT_EQ(copied_message.type, 42U);
    ASSERT_EQ(copied_message.len, sizeof(payload));
    ASSERT_NE(copied_message.buf, nullptr);
    EXPECT_EQ(copied_message.buf[0], 1);
    EXPECT_EQ(copied_message.buf[1], 2);
    EXPECT_EQ(copied_message.buf[2], 3);

    message::Message empty_message(7, nullptr, 0);
    EXPECT_EQ(empty_message.type, 7U);
    EXPECT_EQ(empty_message.len, 0U);
    EXPECT_EQ(empty_message.buf, nullptr);
}

TEST(MessageClientTest, RegistersClearsAndRoutesOnlySubscribedTypes)
{
    message::MessageServer server;
    RecordingClient first("first", &server);
    RecordingClient second("second", &server);
    uint8_t payload[] = {4, 5};

    ASSERT_EQ(first.message_regist(10), RC_OK);
    EXPECT_EQ(first.message_ignore(10), RC_FAIL);
    EXPECT_EQ(first.message_ignore(11), RC_OK);
    ASSERT_EQ(first.message_regist(11), RC_OK);
    ASSERT_EQ(first.message_clear(), RC_OK);
    EXPECT_EQ(first.message_ignore(10), RC_OK);

    ASSERT_EQ(first.message_regist(10), RC_OK);
    ASSERT_EQ(second.message_regist(11), RC_OK);
    ASSERT_EQ(server.add(&first), RC_OK);
    ASSERT_EQ(server.add(&second), RC_OK);

    EXPECT_EQ(first.send_syn(10, payload, sizeof(payload)), RC_OK);
    ASSERT_EQ(first.types.size(), 1U);
    EXPECT_EQ(first.types[0], 10U);
    EXPECT_EQ(first.payloads[0], (std::vector<uint8_t>{4, 5}));
    EXPECT_TRUE(second.types.empty());

    EXPECT_EQ(first.send_syn(NEW_MESSAGE(10, payload, sizeof(payload))), RC_OK);
    ASSERT_EQ(first.types.size(), 2U);
    EXPECT_EQ(first.payloads[1], (std::vector<uint8_t>{4, 5}));

    EXPECT_EQ(first.send_syn(12, payload, sizeof(payload)), RC_UNKNOWN);
    EXPECT_EQ(server.remove(&second), RC_OK);
    EXPECT_EQ(server.remove(&second), RC_UNKNOWN);
}

TEST(MessageServerTest, StopsDispatchingAndRejectsNewClientsAfterStop)
{
    message::MessageServer server;
    RecordingClient client("client", &server);
    ASSERT_EQ(client.message_regist(1), RC_OK);
    ASSERT_EQ(server.add(&client), RC_OK);
    ASSERT_EQ(server.stop(), RC_OK);

    EXPECT_EQ(client.send_syn(1, nullptr, 0), RC_MESSAGE_SERVER_STOPPED);
    EXPECT_EQ(server.add(&client), RC_MESSAGE_SERVER_STOPPED);
    EXPECT_EQ(server.remove(&client), RC_MESSAGE_SERVER_STOPPED);
}

TEST(MessageServerTest, DeliversAsyncMessagesBetweenObserverCallbacks)
{
    message::MessageServer server;
    ASSERT_EQ(server.init(), RC_OK);
    ASSERT_EQ(server.start(), RC_OK);

    std::vector<std::string> events;
    std::mutex events_mutex;
    std::promise<void> send_finished;
    std::future<void> finished = send_finished.get_future();
    RecordingClient sender("sender", &server);
    RecordingClient recipient("recipient", &server, &events, &events_mutex);
    RecordingObserver observer(events, events_mutex, send_finished);
    ASSERT_EQ(recipient.message_regist(20), RC_OK);
    ASSERT_EQ(server.add(&sender), RC_OK);
    ASSERT_EQ(server.add(&recipient), RC_OK);
    EXPECT_EQ(server.add_observer(nullptr), RC_ARG_ERR);
    ASSERT_EQ(server.add_observer(&observer), RC_OK);

    uint8_t payload[] = {8, 9};
    ASSERT_EQ(sender.send(20, payload, sizeof(payload)), RC_OK);
    payload[0] = 0;

    ASSERT_EQ(finished.wait_for(2s), std::future_status::ready);
    EXPECT_EQ(recipient.types, (std::vector<uint64_t>{20}));
    EXPECT_EQ(recipient.payloads, (std::vector<std::vector<uint8_t>>{{8, 9}}));

    {
        std::lock_guard<std::mutex> lock(events_mutex);
        EXPECT_EQ(events, (std::vector<std::string>{"begin", "handler", "finish"}));
    }

    EXPECT_EQ(server.stop(), RC_OK);
    EXPECT_EQ(server.uninit(), RC_OK);
    EXPECT_EQ(sender.send(20, payload, sizeof(payload)), RC_MESSAGE_SERVER_STOPPED);
}

} // namespace
