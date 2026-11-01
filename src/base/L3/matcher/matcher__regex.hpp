
#ifndef regex_matcher_hpp
#define regex_matcher_hpp

#if defined(OS_MACOS)
    #import <Foundation/Foundation.h>
#elif defined(OS_LINUX) || defined(OS_LINUX)
    #include <regex>
#endif

#include <string>

#include "matcher/matcher.hpp"

class RegexMatcher : public Matcher
{
public:
    bool match(const char *i_object) const override;
public:
    bool insert(const std::string &i_pattern) override;
public:
    void clear() override {}
    
private:
#if defined(OS_MACOS)
    NSPredicate* _matcher;
#elif defined(OS_LINUX) || defined(OS_LINUX)
    std::regex _matcher;
#endif
private:
    std::string _pattern;
};

#endif /* regex_matcher_hpp */
