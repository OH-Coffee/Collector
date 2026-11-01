
#ifndef matcher__equal_char_tree_h
#define matcher__equal_char_tree_h

#include "algorithm/trie__char.hpp"
#include "matcher/matcher.hpp"

class EqualCharTreeMatcher : public Matcher
{
public:
    EqualCharTreeMatcher() { _trie = std::make_shared<CharTrie>(); };
public:
    inline bool match(const char        *i_object) const override {
        return this->match( std::string(i_object) );
    }
    inline bool match(const std::string &i_object) const override {
        uint64_t counter = 0; return (_trie->equal(i_object, counter) && (counter == i_object.size()));
    }
public:
    inline bool insert(const std::string &i_pattern) override { return _trie->insert(i_pattern); }
public:
    inline void clear() override { _trie->clear(); }
private:
    std::shared_ptr<CharTrie> _trie{nullptr};
};

#endif /* matcher__equal_char_tree_h */
