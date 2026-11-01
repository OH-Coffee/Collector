
#ifndef matcher_factory_hpp
#define matcher_factory_hpp


#include "matcher/matcher__literal.hpp"
#include "matcher/matcher__literal_extended.hpp"
#include "matcher/matcher__fuzzy.hpp"
#include "matcher/matcher__regex.hpp"
#include "matcher/matcher__not_logic.hpp"

#include "matcher/matcher__str_tree.hpp"
#include "matcher/matcher__forward_path_tree.hpp"
#include "matcher/matcher__reverse_path_tree.hpp"
#include "matcher/matcher__equal_char_tree.hpp"
#include "matcher/matcher__forward_char_tree.hpp"
#include "matcher/matcher__reverse_char_tree.hpp"

typedef enum MatcherType
{
    MATCHER_TYPE__UNKNOWN = 0,
    
    // 基础匹配器
    MATCHER_TYPE__LITERAL,
    MATCHER_TYPE__FUZZY,
    MATCHER_TYPE__REGEX,
    
    // 扩展匹配器
    MATCHER_TYPE__LITERAL_EXT,
    
    MATCHER_TYPE__FORWARD_PATH_TREE,
    MATCHER_TYPE__REVERSE_PATH_TREE,
    
    MATCHER_TYPE__EQUAL_CHAR_TREE,
    MATCHER_TYPE__FORWARD_CHAR_TREE,
    MATCHER_TYPE__REVERSE_CHAR_TREE,
} MatcherType;

class MatcherFactory
{
public:
    static MatcherPtr create(MatcherType i_type);
    static MatcherPtr create(MatcherType i_type, const std::string &i_pattern);
public:
    static MatcherPtr create_not(MatcherType i_type);
    static MatcherPtr create_not(MatcherType i_type, const std::string &i_pattern);
public:
    template<typename T> static MatcherPtr creates(MatcherType i_type, const T &i_patterns, bool i_not=false)
    {
        MatcherPtr r_matcher = ( i_not ? MatcherFactory::create_not(i_type) : MatcherFactory::create(i_type) );
        
        if(nullptr != r_matcher && !i_patterns.empty()) {
            Matcher::insert<T>(r_matcher, i_patterns);
        }
        
        return r_matcher;
    }
};

#endif /* matcher_factory_hpp */

