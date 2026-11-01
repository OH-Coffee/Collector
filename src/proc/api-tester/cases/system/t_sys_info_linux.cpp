#include <fstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "system/sys_info.hpp"
#include "../file/file_test_utils.hpp"

namespace {

TEST(SystemInfoLinuxTest, IteratesMountEntriesAndStopsWhenCallbackRequests)
{
    file_test::TemporaryDirectory test_directory;
    const std::string mount_table_path = test_directory.path("mounts");
    {
        std::ofstream mount_table(mount_table_path);
        ASSERT_TRUE(mount_table.is_open());
        mount_table << "/dev/test0 /mnt/test0 ext4 rw,relatime 0 0\n"
                    << "/dev/test1 /mnt/test1 xfs rw,relatime 0 0\n";
        ASSERT_TRUE(mount_table.good());
    }

    std::vector<std::string> mount_points;
    sys_mounts(mount_table_path, [&mount_points](const struct mntent* mount) {
        mount_points.emplace_back(mount->mnt_dir);
        return mount_points.size() < 1U;
    });

    ASSERT_EQ(mount_points.size(), 1U);
    EXPECT_EQ(mount_points.front(), "/mnt/test0");
}

} // namespace
