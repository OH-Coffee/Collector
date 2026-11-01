
#ifndef matcher__not_logic_hpp
#define matcher__not_logic_hpp

#include "matcher/matcher.hpp"

class NotLogicMatcher : public Matcher
{
public:
             NotLogicMatcher()                     : _matcher(nullptr)   {}
    explicit NotLogicMatcher(MatcherPtr i_matcher) : _matcher(i_matcher) {}
public:
    bool match(const char *i_object) const override;
public:
    void clear()                              override;
    bool insert(const std::string &i_pattern) override;
public:
    void       set_matcher(MatcherPtr i_matcher) { _matcher = i_matcher; }
    MatcherPtr get_matcher() const               { return _matcher;      }
private:
    MatcherPtr _matcher;
};

#endif /* matcher__not_logic_hpp */ 