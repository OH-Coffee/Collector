
#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include <sys/types.h>
#include "header/errcode.hpp"

typedef enum
{
    PID_CACHE_STAGE_UNKNOWN = 0,
    PID_CACHE_STAGE_INSERT,
    PID_CACHE_STAGE_DELETE,
} PidCacheStage;

class PidCacheObserver
{
public:
             PidCacheObserver() = default;
    virtual ~PidCacheObserver() = default;
public:
    virtual ret_code when_pid_insert(pid_t) = 0;
    virtual ret_code when_pid_delete(pid_t) = 0;
};

// PID缓存项
class PidCacheItem
{
public:
     explicit PidCacheItem(size_t);
             ~PidCacheItem();
public:
    ret_code insert(pid_t);
    ret_code clear();
    ret_code is_full();
public:
    ret_code add_observer(PidCacheStage, PidCacheObserver*);
    ret_code del_observer(PidCacheStage, PidCacheObserver*);
private:
    std::atomic_uint    _now_site{};
    std::vector<pid_t>  _pid_cache;
private:
    std::list<PidCacheObserver *> _observers_for_pid_insert;
    std::list<PidCacheObserver *> _observers_for_pid_delete;
private:
    std::mutex _mutex_for_all_attr;
};
typedef std::shared_ptr<PidCacheItem> PidCacheItemPtr;
#define NEW_PID_CACHE_ITEM(i_args) std::make_shared<PidCacheItem>(i_args)

// PID缓存
class PidCache
{
public:
    ret_code set_capacity(uint32_t, uint32_t);
public:
    ret_code insert(pid_t);
    ret_code clear_all_item();
    ret_code clear_full_item();
public:
public:
    ret_code add_observer(PidCacheStage, PidCacheObserver*);
    ret_code del_observer(PidCacheStage, PidCacheObserver*);
private:
    std::atomic_uint64_t            _now_site{0};
    std::vector<PidCacheItemPtr>    _pid_caches;
};