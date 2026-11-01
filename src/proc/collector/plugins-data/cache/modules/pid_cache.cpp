
#include "plugins-data/cache/modules/pid_cache.hpp"

PidCacheItem::PidCacheItem(size_t i_size)
{
    _pid_cache.resize(i_size, -1);
}

PidCacheItem::~PidCacheItem()
{
    this->clear();
}

ret_code PidCacheItem::insert(pid_t i_pid)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_all_attr);
        if(RC_OK == this->is_full()) { break; }

        for(auto observer : _observers_for_pid_insert) {
            observer->when_pid_insert(i_pid);
        }
        _pid_cache[_now_site++] = i_pid; r_code = RC_OK;
    } while (false);

    return r_code;
}

ret_code PidCacheItem::clear()
{
    std::lock_guard<std::mutex> mutex_guard(_mutex_for_all_attr);
    for(pid_t &pid : _pid_cache) {
        for(auto observer : _observers_for_pid_delete) {
            observer->when_pid_delete(pid);
        }
        pid = -1;
    }
    _now_site = 0;

    return RC_OK;
}

ret_code PidCacheItem::is_full()
{
    return (_now_site >= _pid_cache.size()) ? RC_OK : RC_FAIL;
}

ret_code PidCacheItem::add_observer(PidCacheStage i_stage, PidCacheObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(PID_CACHE_STAGE_UNKNOWN == i_stage || nullptr == i_observer) {
            r_code = RC_ARG_ERR; break;
        }

        std::list<PidCacheObserver *> *observers = nullptr;

        if(PID_CACHE_STAGE_INSERT == i_stage) {
            observers = &_observers_for_pid_insert;
        } else if(PID_CACHE_STAGE_DELETE == i_stage) {
            observers = &_observers_for_pid_delete;
        }

        if (nullptr != observers) {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_all_attr);
            for(const auto it : *observers) {
                if(it == i_observer) { goto __SCOPE_END; } // 避免观察者重复
            }
            observers->push_back(i_observer);
        }
        __SCOPE_END: r_code = RC_OK; break;
    } while (false);

    return r_code;
}

ret_code PidCacheItem::del_observer(PidCacheStage i_stage, PidCacheObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(PID_CACHE_STAGE_UNKNOWN == i_stage || nullptr == i_observer) {
            r_code = RC_ARG_ERR; break;
        }

        std::list<PidCacheObserver *> *observers = nullptr;

        if(PID_CACHE_STAGE_INSERT == i_stage) {
            observers = &_observers_for_pid_insert;
        } else if(PID_CACHE_STAGE_DELETE == i_stage) {
            observers = &_observers_for_pid_delete;
        }

        if (nullptr != observers) {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_all_attr);
            for(auto it = observers->begin(); it != observers->end(); it++) {
                if(*it == i_observer) {
                    observers->erase(it); r_code = RC_OK; break;
                }
            }
        }
        if(RC_OK != r_code) { break; }
    } while (false);

    return r_code;
}

ret_code PidCache::set_capacity(uint32_t i_num, uint32_t i_size)
{
    _pid_caches.clear();
    _pid_caches.shrink_to_fit();

    for(uint32_t i = 0; i < i_num; i++) {
        _pid_caches.push_back(NEW_PID_CACHE_ITEM(i_size));
    }

    return RC_OK;
}

ret_code PidCache::insert(pid_t i_pid)
{
    ret_code r_code = RC_UNKNOWN;

    for(size_t i = 0; i < _pid_caches.size(); i++) {
        if(RC_OK == (r_code = _pid_caches[_now_site % _pid_caches.size()]->insert(i_pid))) {
            break;
        }
        ++_now_site;
    }

    return r_code;
}

ret_code PidCache::clear_all_item()
{
    for(const auto &it : _pid_caches) {
        it->clear();
    }
    return RC_OK;
}

ret_code PidCache::clear_full_item()
{
    ret_code r_code = RC_UNKNOWN;

    for(const auto &it : _pid_caches) {
        if(RC_OK == it->is_full()) {
            r_code = it->clear();
        }
    }

    return r_code;
}

ret_code PidCache::add_observer(PidCacheStage i_stage, PidCacheObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    for(const auto &it : _pid_caches) {
        r_code = (RC_OK == it->add_observer(i_stage, i_observer)) ? RC_OK : r_code;
    }

    return r_code;
}

ret_code PidCache::del_observer(PidCacheStage i_stage, PidCacheObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    for(const auto &it : _pid_caches) {
        r_code = (RC_OK == it->del_observer(i_stage, i_observer)) ? RC_OK : r_code;
    }

    return r_code;
}