
#ifndef struct__rule_condition_h
#define struct__rule_condition_h

#include <list>
#include <memory>
#include <string>

namespace rule { /* namespace aegis_mix */

struct Condition; typedef std::shared_ptr<Condition> ConditionPtr;

typedef struct Condition
{
public:
    int64_t id;
    int64_t object;
    int64_t feild;
    int64_t matcher;
    
    std::list<std::string> patterns;
public:
    void clear() {
        this->id        = 0;
        this->object    = -1;
        this->feild     = -1;
        this->matcher   = 0;
        this->patterns.clear();
    }
public:
    template<typename ... T> static ConditionPtr create(T && ... i_args) {
        return std::make_shared<Condition>(std::forward<T>(i_args) ...);
    }
} Condition;


} /* namespace aegis_mix */


#endif /* struct__rule_condition_h */
