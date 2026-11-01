
#ifndef struct__rule_group_h
#define struct__rule_group_h

#include "rule/struct__rule.hpp"

namespace rule { /* namespace aegis_mix */


struct Group; typedef std::shared_ptr<Group> GroupPtr;

typedef struct Group
{
public:
    int64_t     id;
    uint64_t    event;
    
    std::list<RulePtr> rules;
public:
    void clear() {
        this->id    = -1;
        this->event = 0;
        this->rules.clear();
    }
public:
    template<typename ... T> static GroupPtr create(T && ... i_args) {
        return std::make_shared<Group>(std::forward<T>(i_args) ...);
    }
} Group;

} /* namespace aegis_mix */


#endif /* struct__rule_group_h */
