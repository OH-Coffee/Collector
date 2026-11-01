#include <csignal>
#include <string>

#include "gtest/gtest.h"
#include "process/proc_control.hpp"

namespace {

TEST(ProcessControlTest, ReportsCurrentAndInvalidProcessLiveness)
{
    EXPECT_FALSE(proc_not_alive(getpid()));
    EXPECT_TRUE(proc_not_alive(-1));
    EXPECT_TRUE(proc_kill(-1, SIGTERM));
}

TEST(ProcessControlTest, CapturesCommandOutputAndExitStatus)
{
    std::string std_out;
    std::string std_err;

    const uint64_t result = proc_exec_cmd("printf 'stdout'; printf 'stderr' >&2",
                                          std_out,
                                          std_err);

    EXPECT_EQ(result, PROC_EXEC_OK);
    EXPECT_EQ(std_out, "stdout");
    EXPECT_EQ(std_err, "stderr");
}

TEST(ProcessControlTest, PreservesNonzeroCommandExitStatus)
{
    std::string std_out;
    std::string std_err;

    EXPECT_EQ(proc_exec_cmd("exit 7", std_out, std_err), 7U);
}

} // namespace
