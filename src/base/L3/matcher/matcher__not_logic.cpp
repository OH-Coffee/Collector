
#include "matcher/matcher__not_logic.hpp"

bool NotLogicMatcher::match(const char *i_object) const 
{
    return _matcher ? !_matcher->match(i_object) : false;
}

void NotLogicMatcher::clear() 
{
    if(_matcher) { _matcher->clear(); }
}

bool NotLogicMatcher::insert(const std::string &i_pattern) 
{
    return _matcher ? _matcher->insert(i_pattern) : false;
} 
