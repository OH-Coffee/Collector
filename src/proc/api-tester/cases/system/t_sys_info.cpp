#include <ctime>
#include <map>
#include <string>

#include "gtest/gtest.h"
#include "system/sys_info.hpp"

namespace {

TEST(SystemInfoTest, ReturnsKernelAndHardwareIdentifiers)
{
    EXPECT_FALSE(sys_machine().empty());
    EXPECT_FALSE(sys_hostname().empty());
    EXPECT_FALSE(sys_kern_name().empty());
    EXPECT_FALSE(sys_kern_version().empty());
    EXPECT_FALSE(sys_kern_release().empty());
}

TEST(SystemInfoTest, ReturnsPositivePageAndCpuCounts)
{
    EXPECT_GT(sys_page_size(), 0);
    EXPECT_GT(sys_cpu_num(), 0);
}

TEST(SystemInfoTest, ReportsIncreasingMonotonicTime)
{
    const uint64_t first_time = sys_time();
    const uint64_t second_time = sys_time();

    EXPECT_GT(first_time, 0U);
    EXPECT_GE(second_time, first_time);
}

#if defined(OS_LINUX)
TEST(SystemInfoTest, ReturnsSystemBootAndReleaseInformation)
{
    const time_t start_time = sys_start_time();
    const std::map<std::string, std::string> os_release = sys_os_release();

    EXPECT_GT(sys_clock_tick(), 0U);
    EXPECT_GT(start_time, 0);
    EXPECT_LE(start_time, std::time(nullptr));
    EXPECT_FALSE(os_release.empty());
    for (const auto& item : os_release) {
        EXPECT_FALSE(item.first.empty());
    }
}
#endif

} // namespace
