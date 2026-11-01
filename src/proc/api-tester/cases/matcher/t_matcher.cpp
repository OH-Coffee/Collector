#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "matcher/matcher_factory.hpp"

TEST(MatcherTest, LiteralMatchesSubstringAndBoundaries)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__LITERAL, "process");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("running process"));
    EXPECT_TRUE(matcher->match("processor"));

    matcher = MatcherFactory::create(MATCHER_TYPE__LITERAL, "^process");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("process started"));
    EXPECT_FALSE(matcher->match("running process"));

    matcher = MatcherFactory::create(MATCHER_TYPE__LITERAL, "process$");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("running process"));
    EXPECT_FALSE(matcher->match("processor"));
}

TEST(MatcherTest, FuzzyMatchesAnchoredPatternsWithWildcards)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__FUZZY, "^ab?*$");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("ab"));
    EXPECT_TRUE(matcher->match("abxyz"));
    EXPECT_FALSE(matcher->match("acxyz"));
}

TEST(MatcherTest, RegexRequiresTheEntireInputToMatch)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__REGEX, "process[0-9]+");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("process42"));
    EXPECT_FALSE(matcher->match("prefix-process42"));
    EXPECT_FALSE(matcher->match("process"));
}

TEST(MatcherTest, LiteralExtendedMatchesAnyInsertedPattern)
{
    const std::vector<std::string> patterns{"^process", ".log$"};
    auto matcher = MatcherFactory::creates(MATCHER_TYPE__LITERAL_EXT, patterns);
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("process started"));
    EXPECT_TRUE(matcher->match("collector.log"));
    EXPECT_FALSE(matcher->match("collector.txt"));
}

TEST(MatcherTest, ForwardPathTreeMatchesInsertedPathAndDescendants)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__FORWARD_PATH_TREE, "/usr/bin");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("/usr/bin"));
    EXPECT_TRUE(matcher->match("/usr/bin/collector"));
    EXPECT_FALSE(matcher->match("/usr/sbin/collector"));
}

TEST(MatcherTest, ReversePathTreeMatchesInsertedPathSuffix)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__REVERSE_PATH_TREE, "/usr/bin");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("/tmp/usr/bin"));
    EXPECT_FALSE(matcher->match("/tmp/usr/sbin"));
}

TEST(MatcherTest, EqualCharTreeRequiresAnExactMatch)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__EQUAL_CHAR_TREE, "process");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("process"));
    EXPECT_FALSE(matcher->match("processor"));
    EXPECT_FALSE(matcher->match("pro"));
}

TEST(MatcherTest, ForwardCharTreeMatchesInsertedPrefix)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__FORWARD_CHAR_TREE, "process");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("process"));
    EXPECT_TRUE(matcher->match("processor"));
    EXPECT_FALSE(matcher->match("preprocess"));
}

TEST(MatcherTest, ReverseCharTreeMatchesInsertedSuffix)
{
    auto matcher = MatcherFactory::create(MATCHER_TYPE__REVERSE_CHAR_TREE, ".log");
    ASSERT_NE(matcher, nullptr);
    EXPECT_TRUE(matcher->match("collector.log"));
    EXPECT_FALSE(matcher->match("collector.txt"));
}

TEST(MatcherTest, NotMatcherInvertsTheSelectedMatcher)
{
    auto matcher = MatcherFactory::create_not(MATCHER_TYPE__LITERAL, "process");
    ASSERT_NE(matcher, nullptr);
    EXPECT_FALSE(matcher->match("running process"));
    EXPECT_TRUE(matcher->match("collector"));
}
