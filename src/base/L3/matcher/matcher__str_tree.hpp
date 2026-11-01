
#ifndef matcher__string_tree_h
#define matcher__string_tree_h

#include "algorithm/trie.hpp"
#include "matcher/matcher.hpp"

class StringTreeMatcher : public Matcher
{
public:
    StringTreeMatcher(const std::string &i_delim, StringSpliterType i_type) : _delim(i_delim) {
        _trie = std::make_shared<StringTrie>(i_type);
    }
public:
    inline bool match(const char        *i_object) const override { return _trie->find(i_object, _delim); }
    inline bool match(const std::string &i_object) const override { return _trie->find(i_object, _delim); }
public:
    inline bool insert(const std::string &i_pattern) override { return _trie->insert(i_pattern, _delim); }
public:
    inline void clear() override { _trie->clear(); }
private:
    const std::string           _delim;
    std::shared_ptr<StringTrie> _trie{nullptr};
};

#endif /* matcher__string_tree_h */
