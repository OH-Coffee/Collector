#include "gtest/gtest.h"
#include "file/file_type.hpp"
#include "file_test_utils.hpp"
#include "file/file_api.hpp"

namespace {

TEST(FileTypeTest, DetectsRegularFilesAndDirectories)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("sample.txt");
    ASSERT_EQ(file_write(file_path, "plain text"), 10U);

    EXPECT_EQ(file_type(file_path).basic_t, FT_BASIC_REGULAR);
    EXPECT_EQ(file_type(test_directory.path()).basic_t, FT_BASIC_DIR);
    EXPECT_EQ(file_type(test_directory.path("missing")).basic_t, FT_BASIC_UNKNOWN);
}

TEST(FileTypeTest, RespectsRequestedTypeClasses)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("sample.txt");
    ASSERT_EQ(file_write(file_path, "plain text"), 10U);

    const FileType file_type_without_basic = file_type(file_path, FT_CLASS_BINARY);
    EXPECT_EQ(file_type_without_basic.basic_t, FT_BASIC_UNKNOWN);

    const FileType file_type_with_basic = file_type(file_path, FT_CLASS_BASIC);
    EXPECT_EQ(file_type_with_basic.basic_t, FT_BASIC_REGULAR);
    EXPECT_EQ(file_type_with_basic.binary_t, FT_BINARY_UNKNOWN);
}

} // namespace
