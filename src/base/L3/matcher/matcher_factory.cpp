
#include "matcher/matcher_factory.hpp"

MatcherPtr MatcherFactory::create(MatcherType i_type)
{
    MatcherPtr r_matcher = nullptr;
    
    try { switch (i_type) {
        case MATCHER_TYPE__LITERAL:           { r_matcher = std::make_shared<LiteralMatcher>();         } break;
        case MATCHER_TYPE__LITERAL_EXT:       { r_matcher = std::make_shared<LiteralExtendedMatcher>(); } break;
        case MATCHER_TYPE__FUZZY:             { r_matcher = std::make_shared<FuzzyMatcher>();           } break;
        case MATCHER_TYPE__REGEX:             { r_matcher = std::make_shared<RegexMatcher>();           } break;
        case MATCHER_TYPE__FORWARD_PATH_TREE: { r_matcher = std::make_shared<ForwardPathTreeMatcher>(); } break;
        case MATCHER_TYPE__REVERSE_PATH_TREE: { r_matcher = std::make_shared<ReversePathTreeMatcher>(); } break;
        case MATCHER_TYPE__EQUAL_CHAR_TREE:   { r_matcher = std::make_shared<EqualCharTreeMatcher>();   } break;
        case MATCHER_TYPE__FORWARD_CHAR_TREE: { r_matcher = std::make_shared<ForwardCharTreeMatcher>(); } break;
        case MATCHER_TYPE__REVERSE_CHAR_TREE: { r_matcher = std::make_shared<ReverseCharTreeMatcher>(); } break;
        default: {} break;
    } } catch (...) { r_matcher = nullptr; }
    
    return r_matcher;
}

MatcherPtr MatcherFactory::create(MatcherType i_type, const std::string &i_pattern)
{
    MatcherPtr r_matcher = MatcherFactory::create(i_type);
    
    if(nullptr != r_matcher && !i_pattern.empty()) {
        r_matcher->insert(i_pattern);
    }
    
    return r_matcher;
}

MatcherPtr MatcherFactory::create_not(MatcherType i_type)
{
    return std::make_shared<NotLogicMatcher>( MatcherFactory::create(i_type) );
}

MatcherPtr MatcherFactory::create_not(MatcherType i_type, const std::string &i_pattern)
{
    MatcherPtr r_matcher = MatcherFactory::create_not(i_type);

    if(nullptr != r_matcher && !i_pattern.empty()) {
        r_matcher->insert(i_pattern);
    }

    return r_matcher;
}

