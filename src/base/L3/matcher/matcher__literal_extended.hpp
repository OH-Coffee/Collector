
#ifndef matcher__literal_extended_h
#define matcher__literal_extended_h

#include <list>
#include "matcher/matcher__literal.hpp"

class LiteralExtendedMatcher : public Matcher
{
public:
    bool match(const char *i_object) const override {
        for(const auto &matcher : _matchers) {
            if(matcher->match(i_object)) { return true; }
        }
        return false;
    }
public:
    bool insert(const std::string &i_pattern) override {
        if(i_pattern.empty()) { return false; }
        
        MatcherPtr matcher = std::make_shared<LiteralMatcher>();
        if(nullptr != matcher && matcher->insert(i_pattern)) {
            _matchers.emplace_back(matcher); return true;
        }
        return false;
    }
public:
    void clear() override { _matchers.clear(); }
private:
    std::list<MatcherPtr> _matchers;
};

#endif /* matcher__literal_extended_h */
