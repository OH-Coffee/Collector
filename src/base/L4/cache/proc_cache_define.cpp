
#include "cache/proc_cache_define.hpp"

ProcCache::ProcCache()
{
    _enable = false;
}

ProcCache::~ProcCache()
{
    _enable = false;
}

ProcCache& ProcCache::ins()
{
    static ProcCache cache; return cache;
}

ProcInfoPtr ProcCache::find(ProcCacheKey i_key)
{
    ProcInfoPtr r_info = nullptr;
    
    do {
        std::lock_guard<std::mutex> mutex_guard(_mutex);
        
        auto item = _cache.find(i_key.pid);
        if(_cache.end() == item) {
            break;
        }
        
        ProcCacheItemPtr cache_item = item->second;
        
        if(i_key.version <= 0) {
            r_info = cache_item->last_info;
        } else {
            auto info = cache_item->proc_infos.find(i_key.version);
            r_info = (cache_item->proc_infos.end() == info) ? nullptr : info->second;
        }
    } while (false);
    
    if(nullptr != r_info) {
        ++_hit_times;
    } else {
        ++_miss_times;
    }
    
    return r_info;
}

ret_code ProcCache::find(ProcCacheKey i_key, ProcInfoPtr &o_ptr)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        auto ptr = this->find(i_key);
        if(nullptr != ptr) {
            o_ptr = ptr; r_code = RC_OK;
        }
    } while (false);
    
    return r_code;
}

ret_code ProcCache::insert(ProcCacheKey i_key, const ProcInfoPtr& i_data)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(!_enable) { r_code = RC_CACHE_DISABLE; break; }
        
        if(i_key.pid < 0 || i_key.version < 0) {
            r_code = RC_ARG_ERR; break;
        }
        
        std::lock_guard<std::mutex> mutex_guard(_mutex);
        {
            ProcCacheItemPtr cache_item = nullptr;
            
            auto item = _cache.find(i_key.pid);
            if(_cache.end() == item) {
                _cache[i_key.pid] = cache_item = NEW_PROC_CACHE_ITEM();
            } else {
                cache_item = item->second;
            }

            cache_item->proc_infos[i_key.version] = cache_item->last_info = i_data;
            r_code = RC_OK;
        }
    } while (false);
    
    return r_code;
}

ret_code ProcCache::erase(ProcCacheKey i_key)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(!_enable)        { r_code = RC_CACHE_DISABLE; break; }
        if(i_key.pid < 0 )  { r_code = RC_ARG_ERR;       break; }
        
        std::lock_guard<std::mutex> mutex_guard(_mutex);
        {
            _cache.erase(i_key.pid); r_code = RC_OK;
        }
    } while (false);
    
    return r_code;
}

