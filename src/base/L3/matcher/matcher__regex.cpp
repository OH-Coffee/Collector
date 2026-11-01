
#include "matcher/matcher__regex.hpp"

#include <stdexcept>

bool RegexMatcher::match(const char *i_str) const
{
#if defined(OS_MACOS)
    return [_matcher evaluateWithObject:@(i_str)];
#elif defined(OS_LINUX) || defined(OS_LINUX)
    return std::regex_match(i_str, _matcher);
#endif
}

bool RegexMatcher::insert(const std::string &i_pattern)
{
    bool r_bool = true;
    
    do {
#if defined(OS_MACOS)
        NSError *err = nil; NSRegularExpression *reg = nil;
        reg = [NSRegularExpression regularExpressionWithPattern:@(i_pattern.c_str()) options:0 error:&err];
        if(nil == err && nil != reg) {
            _matcher = [NSPredicate predicateWithFormat:@"SELF MATCHES %@", @(i_pattern.c_str())];
        } else {
            throw std::runtime_error("Regex Error");
        }
#elif defined(OS_LINUX) || defined(OS_LINUX)
        _pattern = i_pattern;
        _matcher.assign(_pattern);
#endif
    } while (false);
    
    return r_bool;
}
