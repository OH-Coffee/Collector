
#include "string/string_api.hpp"
#include "filter/filter__executor.hpp"

namespace filter {

MatcherPtr new_matcher(const rule::ConditionPtr &i_rule)
{
    MatcherPtr r_matcher = nullptr;
    
    using MF = MatcherFactory;
    
    switch ( (MatchMode)i_rule->matcher ) {
        case MATCH_MODE__IS: {
            const auto pattern = string_surround(i_rule->patterns.front(), "^", "$");
            r_matcher = MF::create(MATCHER_TYPE__LITERAL, pattern);
        } break;
        case MATCH_MODE__IS_ANY: {
            const auto pattern = string_surround<decltype(i_rule->patterns)>(i_rule->patterns, "^", "$");
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__LITERAL_EXT, pattern);
        } break;

        case MATCH_MODE__NOT: {
            const auto pattern = string_surround(i_rule->patterns.front(), "^", "$");
            r_matcher = MF::create_not(MATCHER_TYPE__LITERAL, pattern);
        } break;
        case MATCH_MODE__NOT_ANY: {
            const auto pattern = string_surround<decltype(i_rule->patterns)>(i_rule->patterns, "^", "$");
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__LITERAL_EXT, pattern, true);
        } break;
            
            
        case MATCH_MODE__CONTAINS: {
            r_matcher = MF::create(MATCHER_TYPE__LITERAL, i_rule->patterns.front());
        } break;
        case MATCH_MODE__CONTAINS_ANY: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__LITERAL_EXT, i_rule->patterns);
        } break;
            
            
        case MATCH_MODE__BEGIN_WITH: {
            const auto pattern = string_surround(i_rule->patterns.front(), "^", "");
            r_matcher = MF::create(MATCHER_TYPE__LITERAL, pattern);
        } break;
        case MATCH_MODE__BEGIN_WITH_ANY: {
            const auto pattern = string_surround<decltype(i_rule->patterns)>(i_rule->patterns, "^", "");
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__LITERAL_EXT, pattern);
        } break;
            
        case MATCH_MODE__END_WITH: {
            const auto pattern = string_surround(i_rule->patterns.front(), "", "$");
            r_matcher = MF::create(MATCHER_TYPE__LITERAL, pattern);
        } break;
        case MATCH_MODE__END_WITH_ANY: {
            const auto pattern = string_surround<decltype(i_rule->patterns)>(i_rule->patterns, "", "$");
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__LITERAL_EXT, pattern);
        } break;
            
        case MATCH_MODE__IS_ANY__CT: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__EQUAL_CHAR_TREE, i_rule->patterns);
        } break;
        case MATCH_MODE__IS_ANY__PT: {
            r_matcher = nullptr; // 暂不支持
        } break;
            
        case MATCH_MODE__BEGIN_WITH_ANY__CT: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__FORWARD_CHAR_TREE, i_rule->patterns);
        } break;
        case MATCH_MODE__BEGIN_WITH_ANY__PT: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__FORWARD_PATH_TREE, i_rule->patterns);
        } break;
            
            
        case MATCH_MODE__END_WITH_ANY__CT: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__REVERSE_CHAR_TREE, i_rule->patterns);
        } break;
        case MATCH_MODE__END_WITH_ANY__PT: {
            r_matcher = MF::creates<decltype(i_rule->patterns)>(MATCHER_TYPE__REVERSE_PATH_TREE, i_rule->patterns);
        } break;
            
        case MATCH_MODE__NOT_CONTAINS: {
            r_matcher = MF::create_not(MATCHER_TYPE__LITERAL, i_rule->patterns.front());
        } break;
            
        default: {} break;
    }
    
    return r_matcher;
}

}
