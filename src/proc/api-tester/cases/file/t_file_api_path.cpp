#include <algorithm>
#include <filesystem>
#include <string>

#include "gtest/gtest.h"
#include "file/file_api.hpp"
#include "file_test_utils.hpp"

namespace {

TEST(FileApiPathTest, ExtractsDirectoryNameAndCanonicalPath)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("sample.txt");
    ASSERT_EQ(file_write(file_path, "data"), 4U);

    EXPECT_EQ(file_dir(file_path), std::filesystem::path(file_path).parent_path().string());
    EXPECT_EQ(file_name(file_path), "sample.txt");
    EXPECT_EQ(file_realpath(file_path), std::filesystem::canonical(file_path).string());
    EXPECT_TRUE(file_realpath(test_directory.path("missing.txt")).empty());
}

TEST(FileApiPathTest, TracesParentPathAndReversesPathComponents)
{
    EXPECT_EQ(path_traceback("/var/tmp/file.txt"), "/var/tmp");
    EXPECT_EQ(path_traceback("filename"), "");
    EXPECT_EQ(path_reverse("/var/tmp/file.txt"), "file.txt/tmp/var");
}

TEST(FileApiPathTest, FindsSuffixOnlyWhenPresentInFileName)
{
    ASSERT_NE(path_suffix("/var/tmp/archive.tar.gz"), nullptr);
    EXPECT_STREQ(path_suffix("/var/tmp/archive.tar.gz"), ".gz");
    EXPECT_EQ(path_suffix("/var/tmp/archive"), nullptr);
    EXPECT_EQ(path_suffix(nullptr), nullptr);
}

TEST(FileApiPathTest, FindsNamesInDirectory)
{
    file_test::TemporaryDirectory test_directory;
    ASSERT_EQ(file_write(test_directory.path("one.txt"), "1"), 1U);
    ASSERT_EQ(file_write(test_directory.path("two.txt"), "2"), 1U);

    const std::vector<std::string> names = file_names_in_dir(test_directory.path());

    EXPECT_EQ(names.size(), 2U);
    EXPECT_NE(std::find(names.begin(), names.end(), "one.txt"), names.end());
    EXPECT_NE(std::find(names.begin(), names.end(), "two.txt"), names.end());
}

} // namespace
