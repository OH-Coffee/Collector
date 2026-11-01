
#ifndef backward_char_tree_matcher_h
#define backward_char_tree_matcher_h

#include <list>

#include "algorithm/trie__char.hpp"
#include "matcher/matcher.hpp"

class ReverseCharTreeMatcher : public Matcher
{
public:
    ReverseCharTreeMatcher() { _trie = std::make_shared<ReverseCharTrie>(); };
public:
    ReverseCharTreeMatcher(const std::list<std::string> &i_patterns) : ReverseCharTreeMatcher() {
        for(const auto &pattern : i_patterns) { this->insert(pattern); }
    }
public:
    inline bool match(const char        *i_object) const override { return _trie->find(i_object); }
    inline bool match(const std::string &i_object) const override { return _trie->find(i_object); }
public:
    inline bool insert(const std::string &i_pattern) override { return _trie->insert(i_pattern); }
public:
    inline void clear() override { _trie->clear(); }
private:
    std::shared_ptr<ReverseCharTrie> _trie{nullptr};
};

#endif /* backward_char_tree_matcher_h */
