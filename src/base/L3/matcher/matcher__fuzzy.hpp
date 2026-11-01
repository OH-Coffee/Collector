
#ifndef fuzzy_matcher_hpp
#define fuzzy_matcher_hpp

#include "matcher/matcher.hpp"

class FuzzyMatcher : public Matcher
{
public:
    bool match(const char *i_object) const override;
public:
    bool insert(const std::string &i_pattern) override { _pattern = i_pattern; return true; }
public:
    void clear() override { _pattern.clear(); }
private:
    bool match_char(const char *i_reg, const char *i_str)               const;
    bool match_star(int i_char, const char *i_reg, const char *i_str)   const;
private:
    std::string _pattern;
};

#endif /* fuzzy_matcher_hpp */

