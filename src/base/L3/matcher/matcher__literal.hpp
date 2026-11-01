
#ifndef literal_matcher_hpp
#define literal_matcher_hpp

#include "matcher/matcher.hpp"

class LiteralMatcher : public Matcher
{
public:
    bool match(const char *i_object) const override;
public:
    bool insert(const std::string &i_pattern) override;
public:
    void clear() override {
        _pattern.clear();
        
        _match_any          = false;
        _match_nil_suffix   = false;
        _match_ends_with    = false;
        _match_begins_with  = false;
    }
private:
    std::string _pattern;
    size_t      _pattern_size;
private:
    bool _match_any         {false};
    bool _match_nil_suffix  {false};
private:
    bool _match_ends_with   {false};
    bool _match_begins_with {false};
};

#endif /* literal_matcher_hpp */

