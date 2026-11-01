#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <thread>

#include "gtest/gtest.h"
#include "algorithm/leaky_bucker.hpp"
#include "algorithm/slider.hpp"

namespace {

TEST(SliderTest, ClampsInitialValueAndMovementToBounds)
{
    Slider slider(5, 0, 10);

    EXPECT_EQ(slider.value(), 5);
    EXPECT_EQ(slider.shl(8), 0);
    EXPECT_EQ(slider.value(), 0);
    EXPECT_EQ(slider >> 12, 10);
    EXPECT_EQ(slider.value(), 10);
    EXPECT_EQ(slider << 3, 7);
    EXPECT_EQ(Slider(-1, 0, 10).value(), 0);
    EXPECT_EQ(Slider(11, 0, 10).value(), 10);
}

TEST(SpeedGovernorTest, ReportsConfigurationAndRejectsExcessCapacity)
{
    SpeedGovernor governor(5, 0);

    EXPECT_EQ(governor.get_size(), std::size_t{5});
    EXPECT_EQ(governor.get_rate(), std::size_t{0});
    EXPECT_TRUE(governor.push(0));
    EXPECT_FALSE(governor.push(1));

    governor.set_size(8);
    governor.set_rate(3);
    EXPECT_EQ(governor.get_size(), std::size_t{8});
    EXPECT_EQ(governor.get_rate(), std::size_t{3});
}

TEST(SpeedGovernorTest, RefillsCapacityAtConfiguredRate)
{
    constexpr std::size_t capacity = 5;
    SpeedGovernor governor(capacity, capacity);
    const std::time_t start = std::time(nullptr);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (std::time(nullptr) <= start &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ASSERT_GT(std::time(nullptr), start)
        << "wall clock did not advance while waiting for governor refill";
    EXPECT_TRUE(governor.push(capacity));
}

} // namespace
