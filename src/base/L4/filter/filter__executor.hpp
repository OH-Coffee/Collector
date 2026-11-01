
#ifndef filter__match_condition_h
#define filter__match_condition_h

#include "matcher/matcher_factory.hpp"
#include "filter/filter__mapping.hpp"


namespace filter {

typedef enum : int64_t
{
    ACTION_UNKNOWN = -1,
    ACTION_INCLUDE = 1,
    ACTION_EXCLUDE = 2,
} Action;

typedef enum : int64_t
{
    RELATIONSHIP_UNKNOWN = -1,
    RELATIONSHIP_AND     = 1,
    RELATIONSHIP_OR      = 2,
} Relationship;

typedef enum MatchMode : int64_t
{
    MATCH_MODE__INVALID            = 0,
    MATCH_MODE__IS                 = 101,
    MATCH_MODE__IS_ANY             = 102,
    MATCH_MODE__CONTAINS           = 103,
    MATCH_MODE__CONTAINS_ANY       = 104,
    MATCH_MODE__NOT                = 105,
    MATCH_MODE__NOT_ANY            = 106,
    MATCH_MODE__NOT_CONTAINS       = 107,
    MATCH_MODE__BEGIN_WITH         = 108,
    MATCH_MODE__BEGIN_WITH_ANY     = 109,
    MATCH_MODE__END_WITH           = 110,
    MATCH_MODE__END_WITH_ANY       = 111,

    MATCH_MODE__IS_ANY__CT         = 201,
    MATCH_MODE__IS_ANY__PT         = 202,
    MATCH_MODE__BEGIN_WITH_ANY__CT = 203,
    MATCH_MODE__BEGIN_WITH_ANY__PT = 204,
    MATCH_MODE__END_WITH_ANY__CT   = 205,
    MATCH_MODE__END_WITH_ANY__PT   = 206,
} MatchMode;

MatcherPtr new_matcher(const rule::ConditionPtr &i_rule);

class Executor; typedef std::shared_ptr<Executor> ExecutorPtr;

class Executor
{
public:
    Executor(const rule::ConditionPtr &i_condition) : _condition(i_condition) {
        _matcher   = new_matcher(i_condition);
        _extractor = new_extractor(i_condition);
    }
public:
    bool match(const StructSet &i_structs) {
        return _matcher->match( _extractor(i_structs) );
    }
public:
    const rule::ConditionPtr &get_condition() { return _condition; }
public:
    template <typename ... T> static ExecutorPtr create(T && ... i_args) {
        return std::make_shared<Executor>(std::forward<T>(i_args) ...);
    }
private:
    MatcherPtr          _matcher;
    FeildExtractor      _extractor;
    rule::ConditionPtr  _condition;
};


}

#endif
