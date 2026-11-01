#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "algorithm/prefix_tree.hpp"
#include "algorithm/trie.hpp"
#include "algorithm/trie__char.hpp"
#include "algorithm/trie__string.hpp"

namespace {

TEST(TrieTest, GenericTrieStoresAndFindsSequences)
{
    Trie<char> trie;
    const std::string word = "cat";
    trie.insert(word.begin(), word.end());

    auto node = trie.find('c', nullptr);
    ASSERT_NE(node, nullptr);
    EXPECT_FALSE(node->is_end());

    node = trie.find('a', node);
    ASSERT_NE(node, nullptr);
    node = trie.find('t', node);
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->is_end());
    EXPECT_EQ(trie.find('x', node), nullptr);
}

TEST(CharTrieTest, MatchesInsertedWordsAndReportsMatchedPrefixLength)
{
    CharTrie trie;
    EXPECT_FALSE(trie.insert(std::string{}));
    ASSERT_TRUE(trie.insert(std::string{"app"}));
    ASSERT_TRUE(trie.insert(std::string{"apple"}));

    EXPECT_TRUE(trie.find("app"));
    EXPECT_TRUE(trie.find("apple"));

    uint64_t matched_length = 0;
    EXPECT_TRUE(trie.find("application", matched_length));
    EXPECT_EQ(matched_length, uint64_t{3});
    EXPECT_FALSE(trie.equal("application", matched_length));
    EXPECT_EQ(matched_length, uint64_t{5});
    EXPECT_FALSE(trie.find("ap", matched_length));
    EXPECT_EQ(matched_length, uint64_t{2});

    trie.clear();
    EXPECT_FALSE(trie.find("app"));
}

TEST(ReverseCharTrieTest, MatchesInsertedSuffixes)
{
    ReverseCharTrie trie;
    EXPECT_FALSE(trie.insert(std::string{}));
    ASSERT_TRUE(trie.insert(std::string{"ing"}));

    uint64_t matched_length = 0;
    EXPECT_TRUE(trie.find("running", matched_length));
    EXPECT_EQ(matched_length, uint64_t{3});
    EXPECT_FALSE(trie.find("run", matched_length));
    EXPECT_EQ(matched_length, uint64_t{0});

    trie.clear();
    EXPECT_FALSE(trie.find("running"));
}

TEST(StringTrieTest, SupportsForwardAndReversePathMatching)
{
    StringTrie forward;
    const std::vector<std::string> paths{"alpha/beta", "alpha/gamma"};
    ASSERT_TRUE(forward.insert(paths, "/"));
    EXPECT_TRUE(forward.find("alpha/beta", "/"));
    EXPECT_TRUE(forward.find("alpha/beta/child", "/"));
    EXPECT_FALSE(forward.find("alpha/delta", "/"));
    EXPECT_FALSE(forward.insert(std::string{}, "/"));

    forward.clear();
    EXPECT_FALSE(forward.find("alpha/beta", "/"));

    StringTrie reverse(STRING_REVERSE_SPLITER);
    ASSERT_TRUE(reverse.insert(std::string{"alpha/beta"}, "/"));
    EXPECT_TRUE(reverse.find("other/alpha/beta", "/"));
    EXPECT_FALSE(reverse.find("other/alpha/gamma", "/"));
}

TEST(PrefixTreeTest, MatchesInsertedPathPrefixes)
{
    PrefixTree tree;
    const std::array<std::string, 2> paths{"root/child", "root/sibling"};

    EXPECT_TRUE(tree.insert(paths));
    EXPECT_TRUE(tree.find("root/child"));
    EXPECT_FALSE(tree.find("root"));
    EXPECT_TRUE(tree.find("root/child/leaf"));
    EXPECT_FALSE(tree.find("root/missing"));
    const std::array<std::string, 1> empty_path{""};
    EXPECT_FALSE(tree.insert(empty_path));
}

} // namespace
