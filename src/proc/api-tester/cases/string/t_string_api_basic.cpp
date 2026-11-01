#include <array>
#include <cstdint>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "string/string_api.hpp"

namespace {

TEST(StringApiBasicTest, CopiesStringsIntoCharBuffers)
{
    const std::string in_string = "collector";
    std::array<char, 5> out_buffer{};

    string_copy2(in_string, out_buffer.size(), out_buffer.data());

    EXPECT_EQ(std::string(out_buffer.data(), out_buffer.size()), "colle");
}

TEST(StringApiBasicTest, CopiesStringsIntoByteBuffers)
{
    const std::string in_string = "test";
    std::array<uint8_t, 8> out_buffer{};

    string_copy2(in_string, out_buffer.size(), out_buffer.data());

    EXPECT_EQ(std::string(out_buffer.begin(), out_buffer.begin() + in_string.size()), in_string);
    EXPECT_EQ(out_buffer[in_string.size()], 0);
}

TEST(StringApiBasicTest, DetectsDigitStrings)
{
    EXPECT_TRUE(string_is_digit("0123456789"));
    EXPECT_FALSE(string_is_digit("123a"));

    const char in_buffer[] = {'1', '2', '3'};
    EXPECT_TRUE(string_is_digit(in_buffer, sizeof(in_buffer)));
    EXPECT_FALSE(string_is_digit(nullptr, 1));
}

TEST(StringApiBasicTest, ChecksStringPrefixes)
{
    EXPECT_TRUE(string_start_with("  \tvalue", 'v'));
    EXPECT_FALSE(string_start_with("value", 'a'));
    EXPECT_TRUE(string_start_with("collector", std::string("collect")));
    EXPECT_FALSE(string_start_with("collector", std::string("Collector")));
}

TEST(StringApiBasicTest, FindsFirstNonSpaceSubstring)
{
    EXPECT_EQ(string_first_no_space_substr(" \tvalue remainder"), "value");
    EXPECT_EQ(string_first_no_space_substr("   "), "");
    EXPECT_EQ(string_first_no_space_substr(""), "");
}

TEST(StringApiBasicTest, SplitsPairsAndTrimsValuesWhenRequested)
{
    const std::string in_string = " key : value remainder";

    EXPECT_EQ(string_pair(in_string, ":", false),
              (std::pair<std::string, std::string>{" key ", " value remainder"}));
    EXPECT_EQ(string_pair(in_string, ":", true),
              (std::pair<std::string, std::string>{"key", "value"}));
    EXPECT_EQ(string_pair_key(in_string, ":", true), "key");
    EXPECT_EQ(string_pair_val(in_string, ":", true), "value");
    EXPECT_EQ(string_pair("missing delimiter", ":", false),
              (std::pair<std::string, std::string>{"missing delimiter", ""}));
}

} // namespace
