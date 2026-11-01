#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "string/string_api.hpp"

namespace {

TEST(StringApiConvertTest, ConvertsIntegerStrings)
{
    EXPECT_EQ(string_to_integer<int>("42"), 42);
    EXPECT_EQ(string_to_integer<int>("-42"), -42);
    EXPECT_EQ(string_to_integer<unsigned int>("0x2a"), 42U);
}

TEST(StringApiConvertTest, JoinsStringContainers)
{
    EXPECT_EQ(string_joint(std::vector<std::string>{"one", "two", "three"}, ','), "one,two,three");
    EXPECT_EQ(string_joint(std::list<std::string>{"one", "two"}), "one two");
    EXPECT_EQ(string_joint(std::set<std::string>{"one", "two"}, '-'), "one-two");
    EXPECT_EQ(string_joint(std::vector<std::string>{}), "");
}

TEST(StringApiConvertTest, ConvertsStringsToBinaryAndHexadecimal)
{
    const std::string in_string{"A", 1};

    EXPECT_EQ(string_str2bin(in_string), "01000001");
    EXPECT_EQ(string_str2bin(in_string.data(), in_string.size()), "01000001");
    EXPECT_EQ(string_str2hex(in_string), "41");
    EXPECT_EQ(string_str2hex(in_string.data(), in_string.size()), "41");
    EXPECT_EQ(string_int2oct(0), "0");
    EXPECT_EQ(string_int2oct(83), "123");
    EXPECT_EQ(string_int2oct(-8), "-10");
}

TEST(StringApiConvertTest, ConvertsBinaryAndHexadecimalToRawBytes)
{
    std::array<char, 2> out_buffer{};

    EXPECT_EQ(string_bin2raw("0100000101000010", out_buffer.size(), out_buffer.data()), 2U);
    EXPECT_EQ(out_buffer[0], 'A');
    EXPECT_EQ(out_buffer[1], 'B');

    EXPECT_EQ(string_hex2raw("4142", out_buffer.size(), out_buffer.data()), 2U);
    EXPECT_EQ(out_buffer[0], 'A');
    EXPECT_EQ(out_buffer[1], 'B');

    EXPECT_EQ(string_hex2raw("4142", 1, out_buffer.data()), 1U);
    EXPECT_EQ(out_buffer[0], 'A');
}

} // namespace
