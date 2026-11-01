#include <set>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "string/string_api.hpp"

namespace {

TEST(StringApiTransformTest, ReversesDelimitedItems)
{
    EXPECT_EQ(string_reverse("one,two,three", ","), "three,two,one");
    EXPECT_EQ(string_reverse("", ","), "");
}

TEST(StringApiTransformTest, ReplacesStringAndSetAlternatives)
{
    EXPECT_EQ(string_replace("one fish, two fish", "fish", "bird"),
              "one bird, two bird");
    EXPECT_EQ(string_replace("a-b-a", "a", std::set<std::string>{"x", "y"}),
              (std::set<std::string>{"x-b-a", "y-b-a"}));
}

TEST(StringApiTransformTest, ReplacesAllOccurrencesRecursively)
{
    EXPECT_EQ(string_replace_recursion("a+a", "a", std::set<std::string>{"x", "y"}),
              (std::set<std::string>{"x+x", "x+y", "y+x", "y+y"}));
}

TEST(StringApiTransformTest, ChangesLetterCaseAndDropsQuotes)
{
    EXPECT_EQ(string_to_lower("AbC-123"), "abc-123");
    EXPECT_EQ(string_to_upper("AbC-123"), "ABC-123");
    EXPECT_EQ(string_drop_quotes("\"value\""), "value");
    EXPECT_EQ(string_drop_quotes("'value'"), "value");
    EXPECT_EQ(string_drop_quotes("value"), "value");
}

TEST(StringApiTransformTest, AddsPrefixesSuffixesAndSurroundsStrings)
{
    EXPECT_EQ(string_add_prefix("value", "pre-"), "pre-value");
    EXPECT_EQ(string_add_suffix("value", "-post"), "value-post");
    EXPECT_EQ(string_surround("value", "[", "]"), "[value]");
}

TEST(StringApiTransformTest, SurroundsContainerItems)
{
    const std::vector<std::string> in_strings{"one", "two"};

    EXPECT_EQ(string_surround(in_strings, "<", ">"),
              (std::vector<std::string>{"<one>", "<two>"}));
}

} // namespace
