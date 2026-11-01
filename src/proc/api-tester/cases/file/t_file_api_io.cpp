#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "file/file_api.hpp"
#include "file_test_utils.hpp"

namespace {

TEST(FileApiIoTest, CreatesChecksSizesReadsAndRemovesFiles)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("nested/data.txt");

    EXPECT_EQ(file_is_exist(file_path), RC_FILE_NO_EXIST);
    EXPECT_EQ(file_create(file_path), RC_OK);
    EXPECT_EQ(file_is_exist(file_path), RC_OK);
    EXPECT_EQ(file_size(file_path), 0U);

    ASSERT_EQ(file_write(file_path, "collector"), 9U);
    EXPECT_EQ(file_size(file_path), 9U);

    std::array<char, 5> buffer{};
    EXPECT_EQ(file_read(file_path, buffer.size(), buffer.data()), 5U);
    EXPECT_EQ(std::string(buffer.data(), buffer.size()), "colle");
    EXPECT_EQ(file_read_all_s(file_path), "collector");
    EXPECT_EQ(file_read_all_v(file_path),
              (std::vector<uint8_t>{'c', 'o', 'l', 'l', 'e', 'c', 't', 'o', 'r'}));

    EXPECT_EQ(file_remove(file_path), RC_OK);
    EXPECT_EQ(file_is_exist(file_path), RC_FILE_NO_EXIST);
}

TEST(FileApiIoTest, AppendsStringContents)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("append.txt");

    ASSERT_EQ(file_write(file_path, "first"), 5U);
    EXPECT_EQ(file_append(file_path, "-second"), 12U);
    EXPECT_EQ(file_read_all_s(file_path), "first-second");
}

TEST(FileApiIoTest, ReadsLinesWithLimitAndCallback)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("lines.txt");
    ASSERT_EQ(file_write(file_path, "one\ntwo\nthree\n"), 14U);

    EXPECT_EQ(file_read_lines(file_path, 2),
              (std::vector<std::string>{"one", "two"}));

    std::vector<std::string> callback_lines;
    file_read_lines(file_path, [&callback_lines](const std::string& i_line) {
        callback_lines.push_back(i_line);
        return callback_lines.size() < 2;
    });
    EXPECT_EQ(callback_lines, (std::vector<std::string>{"one", "two"}));
}

TEST(FileApiIoTest, SavesAndReadsIntegerValues)
{
    file_test::TemporaryDirectory test_directory;
    const std::string file_path = test_directory.path("number.txt");
    ASSERT_EQ(file_save_num(-12345, file_path.c_str()), RC_OK);

    EXPECT_EQ(file_read_num(file_path), -12345);

    int32_t number = 0;
    EXPECT_EQ(file_read_num(file_path.c_str(), number), RC_OK);
    EXPECT_EQ(number, -12345);
}

TEST(FileApiIoTest, RejectsInvalidPathsAndBuffers)
{
    file_test::TemporaryDirectory test_directory;
    std::array<char, 4> buffer{};

    EXPECT_EQ(file_create("relative/path.txt"), RC_UNKNOWN);
    EXPECT_EQ(file_create(""), RC_UNKNOWN);
    EXPECT_EQ(file_read("", buffer.size(), buffer.data()), 0U);
    EXPECT_EQ(file_read(test_directory.path("missing"), buffer.size(), buffer.data()), 0U);
    EXPECT_EQ(file_read(test_directory.path("missing"), buffer.size(), nullptr), 0U);
}

} // namespace
