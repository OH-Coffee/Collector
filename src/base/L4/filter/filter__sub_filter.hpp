
#ifndef filter__sub_filter_h
#define filter__sub_filter_h

#include "rule/loader.hpp"
#include "filter/filter__resolver.hpp"

namespace filter {

class SubFilter; typedef std::shared_ptr<SubFilter> SubFilterPtr;

class SubFilter
{
public:
     SubFilter() = default;
    ~SubFilter() = default;
public:
    ret_code load(const rule::Loader &i_loader);
public:
    ret_code reload(rule::Loader &i_loader) {
        this->clear(); return this->load(i_loader);
    }
public:
    ret_code match(uint64_t i_event, const StructSet &i_structs);
    ret_code match(uint64_t i_event, const StructSet &i_structs, std::string &o_id);
public:
    void       clear() { _filters.clear();              }
    ret_code  enable() { _enable = true;  return RC_OK; }
    ret_code disable() { _enable = false; return RC_OK; }
public:
    template<typename ... T> static SubFilterPtr create(T && ... i_args) {
        return std::make_shared<SubFilter>( std::forward<T>(i_args)... );
    }
public:
    inline bool enable_default_on_match() const         { return _enable_default_on_match;      };
    inline void enable_default_on_match(bool i_enable)  { _enable_default_on_match = i_enable;  };
private:
    std::atomic_bool _enable{false};
    std::atomic_bool _enable_default_on_match{false};
private:
    std::map<int64_t, ResolverGroup> _filters;
};

}

#endif
