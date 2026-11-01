
#ifndef proc_cache_hpp
#define proc_cache_hpp

#include "cache/proc_cache_struct.hpp"

class ProcCache
{
private:
    ProcCache();
public:
    ~ProcCache();
public:
    static ProcCache& ins();
public:
    ret_code init();
public:
    ret_code  enable() { _enable = true;  return RC_OK; }
    ret_code disable() { _enable = false; return RC_OK; }
public:
    ProcInfoPtr find(ProcCacheKey i_key);
    ret_code find(ProcCacheKey i_key, ProcInfoPtr &o_ptr);
public:
    ret_code erase(ProcCacheKey i_key);
    ret_code insert(ProcCacheKey i_key, const ProcInfoPtr& i_data);
public:
    const auto &get() { return _cache; }
private:
    std::atomic_bool                  _enable{};
private:
    std::mutex                        _mutex;
    std::map<pid_t, ProcCacheItemPtr> _cache;
private:
    std::atomic_uint64_t _hit_times{};
    std::atomic_uint64_t _miss_times{};
};


#endif /* proc_cache_hpp */

