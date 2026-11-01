
#ifndef matcher__reverse_str_tree_h
#define matcher__reverse_str_tree_h

#include "matcher/matcher__str_tree.hpp"

class ReversePathTreeMatcher : public StringTreeMatcher
{
public:
    ReversePathTreeMatcher() : StringTreeMatcher("/", STRING_REVERSE_SPLITER) {}
};

#endif /* matcher__reverse_str_tree_h */
