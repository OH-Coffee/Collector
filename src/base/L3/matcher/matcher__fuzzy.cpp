
#include "matcher/matcher__fuzzy.hpp"

bool FuzzyMatcher::match(const char *i_str) const
{
    if(_pattern.empty()) { return false; }

    const char *reg = _pattern.c_str();
    if(reg[0] == '^') {
        return match_char(reg + 1, i_str);
    }
    
    do {
        if(match_char(reg, i_str)) { return true; }
    } while (*i_str++ != '\0');
    
    return false;
}

bool FuzzyMatcher::match_char(const char *i_reg, const char *i_str) const
{
    if(i_reg[0] == '\0') { return true; }
    
    if(i_reg[1] == '*') {
        return match_star(i_reg[0], i_reg + 2, i_str);
    }
    if(i_reg[0] == '$' && i_reg[1] == '\0') {
        return *i_str == '\0';
    }
    if(*i_str != '\0' && (i_reg[0] == '?' || i_reg[0] == *i_str)) {
        return match_char(i_reg + 1, i_str + 1);
    }
    
    return false;
}

bool FuzzyMatcher::match_star(int i_char, const char *i_reg, const char *i_str) const
{
    do {
        if(match_char(i_reg, i_str)) {
            return true;
        }
    } while (*i_str != '\0' && (*i_str++ == i_char || i_char == '?'));
    
    return false;
}

