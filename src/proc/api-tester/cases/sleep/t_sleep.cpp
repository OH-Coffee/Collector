#include <array>
#include <algorithm>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "sleep/sleep.hpp"

namespace {

bool wake_until_ready(Sleep& i_sleep,
                      std::vector<std::future<ret_code>>& i_results,
                      std::chrono::milliseconds i_timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + i_timeout;

    while(std::chrono::steady_clock::now() < deadline) {
        const bool all_ready = std::all_of(i_results.begin(), i_results.end(), [](auto& i_result) {
            return i_result.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
        });
        if(all_ready) { return true; }

        i_sleep.wakeup();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    return std::all_of(i_results.begin(), i_results.end(), [](auto& i_result) {
        return i_result.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    });
}

TEST(SleepTest, WakeupInterruptsActiveSleep)
{
    Sleep sleep;
    std::promise<void> sleep_started;
    std::vector<std::future<ret_code>> sleep_results;
    sleep_results.emplace_back(std::async(std::launch::async, [&] {
        sleep_started.set_value();
        return sleep.sleep_ms(5000);
    }));

    sleep_started.get_future().wait();

    ASSERT_TRUE(wake_until_ready(sleep, sleep_results, std::chrono::seconds(1)));
    EXPECT_EQ(sleep_results.front().get(), RC_OK);
}

TEST(SleepTest, WakeupInterruptsAllActiveSleepers)
{
    Sleep sleep;
    std::array<std::promise<void>, 3> sleep_started;
    std::vector<std::future<ret_code>> sleep_results;
    sleep_results.reserve(sleep_started.size());

    for (std::size_t i = 0; i < sleep_started.size(); ++i) {
        sleep_results.emplace_back(std::async(std::launch::async, [&sleep, &sleep_started, i] {
            sleep_started[i].set_value();
            return sleep.sleep_ms(5000);
        }));
    }

    for (auto& started : sleep_started) {
        started.get_future().wait();
    }

    ASSERT_TRUE(wake_until_ready(sleep, sleep_results, std::chrono::seconds(1)));
    for (auto& result : sleep_results) {
        EXPECT_EQ(result.get(), RC_OK);
    }
}

TEST(SleepTest, WakeupWithoutWaitersIsNotRemembered)
{
    Sleep sleep;
    ASSERT_EQ(sleep.wakeup(), RC_OK);

    const auto start = std::chrono::steady_clock::now();
    EXPECT_EQ(sleep.sleep_ms(100), RC_OK);
    EXPECT_GE(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(80));
}

TEST(SleepTest, SleepTimesOutWithoutWakeup)
{
    Sleep sleep;

    EXPECT_EQ(sleep.sleep_ms(1), RC_OK);
}

TEST(SleepTest, WakeupInterruptsSecondBasedSleep)
{
    Sleep sleep;
    std::promise<void> sleep_started;
    std::vector<std::future<ret_code>> sleep_results;
    sleep_results.emplace_back(std::async(std::launch::async, [&] {
        sleep_started.set_value();
        return sleep.sleep(5);
    }));

    sleep_started.get_future().wait();

    ASSERT_TRUE(wake_until_ready(sleep, sleep_results, std::chrono::seconds(1)));
    EXPECT_EQ(sleep_results.front().get(), RC_OK);
}

} // namespace
