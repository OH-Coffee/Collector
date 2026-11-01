
#ifndef matcher__forward_str_tree_h
#define matcher__forward_str_tree_h

#include "matcher/matcher__str_tree.hpp"

class ForwardPathTreeMatcher : public StringTreeMatcher
{
public:
    ForwardPathTreeMatcher() : StringTreeMatcher("/", STRING_FORWARD_SPLITER) {}
};


#endif /* matcher__forward_str_tree_h */
