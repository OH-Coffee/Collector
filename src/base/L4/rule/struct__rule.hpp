
#ifndef struct__rule_h
#define struct__rule_h

#include "header/errcode.hpp"
#include "rule/struct__condition.hpp"

namespace rule { /* namespace aegis_mix */

struct Rule; typedef std::shared_ptr<Rule> RulePtr;

typedef struct Rule
{
public:
    int64_t id;
    int64_t action;
    int64_t relationship;
    
    std::list<ConditionPtr> conditions;
public:
    void clear() {
        this->id            = -1;
        this->action        = -1;
        this->relationship  = -1;
        this->conditions.clear();
    }
public:
    template<typename ... T> static RulePtr create(T && ... i_args) {
        return std::make_shared<Rule>(std::forward<T>(i_args) ...);
    }
} Rule;


} /* namespace aegis_mix */


#endif /* struct__rule_h */
