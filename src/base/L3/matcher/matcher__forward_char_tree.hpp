
#ifndef forward_char_tree_matcher_h
#define forward_char_tree_matcher_h

#include "algorithm/trie__char.hpp"
#include "matcher/matcher.hpp"

class ForwardCharTreeMatcher : public Matcher
{
public:
    ForwardCharTreeMatcher() { _trie = std::make_shared<CharTrie>(); };
public:
    inline bool match(const char        *i_object) const override { return _trie->find(i_object); }
    inline bool match(const std::string &i_object) const override { return _trie->find(i_object); }
public:
    inline bool insert(const std::string &i_pattern) override { return _trie->insert(i_pattern); }
public:
    inline void clear() override { _trie->clear(); }
private:
    std::shared_ptr<CharTrie> _trie{nullptr};
};

#endif /* forward_char_tree_matcher_h */
