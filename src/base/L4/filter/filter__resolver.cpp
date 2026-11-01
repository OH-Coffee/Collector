
#include "filter/filter__resolver.hpp"

namespace filter {

bool Resolver::match(const StructSet &i_structs)
{
    if(RELATIONSHIP_AND == _rule->relationship) {
        return match_for_condition_relation__and(i_structs);
    } else if(RELATIONSHIP_OR == _rule->relationship) {
        return match_for_condition_relation__or(i_structs);
    }
    
    return false;
}

inline bool Resolver::match_for_condition_relation__or(const StructSet &i_structs)
{
    for(const auto &executor : _executors) {
        if(executor->match(i_structs)) { return true; }
    }
    return false;
}

inline bool Resolver::match_for_condition_relation__and(const StructSet &i_structs)
{
    for(const auto &executor : _executors) {
        if(!executor->match(i_structs)) { return false; }
    }
    return true;
}

bool Resolver::match(const StructSet &i_structs, int64_t &o_id)
{
    if(RELATIONSHIP_AND == _rule->relationship) {
        for(const auto &executor : _executors) {
            if(!executor->match(i_structs)) { o_id = executor->get_condition()->id; return false; }
        }
        return true;
    } else if(RELATIONSHIP_OR == _rule->relationship) {
        for(const auto &executor : _executors) {
            if(executor->match(i_structs)) { o_id = executor->get_condition()->id; return true; }
        }
        return false;
    }
    
    return false;
}

}

