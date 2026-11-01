#include <algorithm>
#include <array>
#include <cstdlib>
#include <string>
#include <vector>

#include <unistd.h>

#include "gtest/gtest.h"
#include "process/proc_info.hpp"

namespace {

TEST(ProcessInfoTest, ReadsCurrentProcessNameAndPaths)
{
    const pid_t self_pid = getpid();

    EXPECT_FALSE(proc_name(self_pid).empty());
    EXPECT_FALSE(proc_path(self_pid).empty());
    EXPECT_FALSE(proc_work_dir(self_pid).empty());
    EXPECT_FALSE(proc_root_dir(self_pid).empty());

    std::array<char, 256> name_buffer{};
    EXPECT_EQ(proc_name(self_pid, name_buffer.size(), name_buffer.data()), name_buffer.data());
    EXPECT_STREQ(name_buffer.data(), proc_name(self_pid).c_str());
}

TEST(ProcessInfoTest, ReadsCurrentProcessStatAndStatus)
{
    const pid_t self_pid = getpid();
    const std::vector<std::string> stat = proc_stat(self_pid);
    const std::map<std::string, std::string> status = proc_status(self_pid);

    ASSERT_GT(stat.size(), PROC_STAT_SYS_TIME);
    EXPECT_EQ(stat[PROC_STAT_PID], std::to_string(self_pid));
    EXPECT_FALSE(stat[PROC_STAT_NAME].empty());
    EXPECT_NE(status.find("Name"), status.end());
    EXPECT_NE(status.find("VmRSS"), status.end());
}

TEST(ProcessInfoTest, FindsCurrentProcessByPidNameAndExecutablePath)
{
    const pid_t self_pid = getpid();
    std::array<pid_t, 4096> pid_buffer{};
    const int pid_count = proc_all_pids(pid_buffer.data(), pid_buffer.size());

    ASSERT_GT(pid_count, 0);
    EXPECT_NE(std::find(pid_buffer.begin(), pid_buffer.begin() + pid_count, self_pid),
              pid_buffer.begin() + pid_count);

    std::vector<pid_t> matching_pids;
    EXPECT_GT(proc_pids_by_name(proc_name(self_pid).c_str(), matching_pids), 0);
    EXPECT_NE(std::find(matching_pids.begin(), matching_pids.end(), self_pid),
              matching_pids.end());

    matching_pids.clear();
    EXPECT_GT(proc_pids_by_path(proc_path(self_pid).c_str(), matching_pids), 0);
    EXPECT_NE(std::find(matching_pids.begin(), matching_pids.end(), self_pid),
              matching_pids.end());
}

TEST(ProcessInfoTest, ReadsArgumentsAndMemoryInformation)
{
    const pid_t self_pid = getpid();
    const std::vector<std::string> args = proc_args_info(
        self_pid, static_cast<ProcArgsInfo>(PROC_ARG_ARGC | PROC_ARG_ARGV));

    ASSERT_GE(args.size(), 2U);
    EXPECT_GT(std::strtol(args.front().c_str(), nullptr, 10), 0);
    EXPECT_FALSE(proc_cmdline(self_pid).empty());
    EXPECT_FALSE(proc_args(self_pid).empty());
    EXPECT_GT(proc_opened_fd_cnt(self_pid), 0U);
    EXPECT_GT(proc_self_opened_fd_cnt(), 0U);
    EXPECT_GT(proc_rss_mem(self_pid), 0U);
}

TEST(ProcessInfoTest, ReadsCurrentProcessThreads)
{
    std::vector<uint64_t> thread_ids;

    EXPECT_TRUE(proc_threads(getpid(), thread_ids));
    EXPECT_NE(std::find(thread_ids.begin(), thread_ids.end(), static_cast<uint64_t>(getpid())),
              thread_ids.end());
}

TEST(ProcessInfoTest, ChecksFlagMasks)
{
    EXPECT_TRUE(proc_flags_is_set(0x0f, 0x03));
    EXPECT_FALSE(proc_flags_is_set(0x01, 0x03));
    EXPECT_TRUE(proc_flags_is_set(0x01, 0x00));
}

} // namespace
