
#ifndef filter__match_rule_h
#define filter__match_rule_h


#include "cache/proc_cache_struct.hpp"

#include "matcher/matcher_factory.hpp"
#include "filter/filter__executor.hpp"


namespace filter { /* namespace aegis_mix */

class Resolver; typedef std::shared_ptr<Resolver> ResolverPtr;

struct ResolverGroup
{
    Action                  on_match_default;
    rule::GroupPtr          rule_group;
    std::list<ResolverPtr>  resolvers;
};

class Resolver
{
public:
    Resolver(const rule::RulePtr &i_rule) : _rule(i_rule) {
        for(const auto &condition : _rule->conditions) {
            _executors.emplace_back( Executor::create(condition) );
        }
    }
public:
    bool match(const StructSet &i_structs);
    bool match(const StructSet &i_structs, int64_t &o_id);
public:
    const rule::RulePtr &get_rule() { return _rule; }
private:
    inline bool match_for_condition_relation__or(const StructSet &i_structs);
    inline bool match_for_condition_relation__and(const StructSet &i_structs);
public:
    template<typename ... T> static ResolverPtr create(T && ... i_args) {
        return std::make_shared<Resolver>( std::forward<T>(i_args)... );
    }
private:
    rule::RulePtr _rule;
private:
    std::list<ExecutorPtr> _executors;
};

/* ---------------------------------------------------------------------------------------------------- */

} /* namespace aegis_mix */


#endif /* filter__match_rule_h */
