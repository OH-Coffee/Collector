#include <chrono>
#include <future>
#include <thread>

#include "gtest/gtest.h"
#include "thread/thread_pool.hpp"

namespace {

using namespace std::chrono_literals;

TEST(ThreadPoolTest, InitializesAndConfiguresWorkerAndQueueLimits)
{
    thread::ThreadPool pool;

    EXPECT_EQ(pool.init(), RC_OK);
    EXPECT_EQ(pool.get_thread_size(), DEFAULT_THREAD_SIZE);
    EXPECT_EQ(pool.set_thread_size(2), 2U);
    EXPECT_EQ(pool.get_thread_size(), 2U);
    EXPECT_EQ(pool.set_queue_size(10), 10U);
    EXPECT_EQ(pool.set_task_interval(25), 25U);
    EXPECT_EQ(pool.uninit(), RC_OK);
}

TEST(ThreadPoolTest, ExecutesQueuedTaskAndRejectsTasksAfterStop)
{
    thread::ThreadPool pool;
    ASSERT_EQ(pool.set_thread_size(1), 1U);
    ASSERT_EQ(pool.set_queue_size(1), 1U);

    std::promise<int> task_result;
    std::future<int> result = task_result.get_future();
    EXPECT_EQ(pool.add_task([](std::promise<int>& promise, int value) {
        promise.set_value(value);
    }, std::ref(task_result), 42), RC_OK);

    EXPECT_EQ(pool.is_full(), RC_OK);
    EXPECT_EQ(pool.add_task([] {}), RC_THREAD_POLL_TASK_QUEUE_FULL);
    ASSERT_EQ(pool.start(), RC_OK);
    ASSERT_EQ(result.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(result.get(), 42);

    EXPECT_EQ(pool.stop(), RC_OK);
    EXPECT_EQ(pool.add_task([] {}), RC_THREAD_POLL_STOP);
}

TEST(ThreadPoolTest, PreservesForwardedArgumentValueCategories)
{
    thread::ThreadPool pool;
    ASSERT_EQ(pool.set_thread_size(1), 1U);

    int lvalue = 1;
    std::promise<bool> lvalue_category;
    std::future<bool> lvalue_result = lvalue_category.get_future();
    ASSERT_EQ(pool.add_task([&lvalue_category](auto&& value) {
        lvalue_category.set_value(std::is_lvalue_reference_v<decltype(value)>);
    }, lvalue), RC_OK);

    std::promise<bool> rvalue_category;
    std::future<bool> rvalue_result = rvalue_category.get_future();
    ASSERT_EQ(pool.add_task([&rvalue_category](auto&& value) {
        rvalue_category.set_value(std::is_rvalue_reference_v<decltype(value)>);
    }, 2), RC_OK);

    ASSERT_EQ(pool.start(), RC_OK);
    ASSERT_EQ(lvalue_result.wait_for(1s), std::future_status::ready);
    ASSERT_EQ(rvalue_result.wait_for(1s), std::future_status::ready);
    EXPECT_TRUE(lvalue_result.get());
    EXPECT_TRUE(rvalue_result.get());
    EXPECT_EQ(pool.stop(), RC_OK);
}

TEST(ThreadPoolTest, PausesTaskExecutionUntilResumed)
{
    thread::ThreadPool pool;
    ASSERT_EQ(pool.set_thread_size(1), 1U);
    ASSERT_EQ(pool.pause(), RC_OK);
    ASSERT_EQ(pool.start(), RC_OK);

    std::promise<void> task_completed;
    std::future<void> completion = task_completed.get_future();
    ASSERT_EQ(pool.add_task([&task_completed] {
        task_completed.set_value();
    }), RC_OK);

    EXPECT_EQ(completion.wait_for(50ms), std::future_status::timeout);
    ASSERT_EQ(pool.resume(), RC_OK);
    EXPECT_EQ(completion.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(pool.stop(), RC_OK);
}

TEST(ThreadPoolTest, StopWakesIdleWorkers)
{
    thread::ThreadPool pool;
    ASSERT_EQ(pool.set_thread_size(2), 2U);
    ASSERT_EQ(pool.start(), RC_OK);
    EXPECT_EQ(pool.stop(), RC_OK);
    EXPECT_EQ(pool.stop(), RC_OK);
}

} // namespace
