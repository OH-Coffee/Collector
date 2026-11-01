
#include "cache/proc_cache_struct.hpp"

ProcCacheItem::ProcCacheItem()
{
    this->last_time = 0;
    this->last_info = nullptr;
}


ProcCacheKey::ProcCacheKey(pid_t i_pid, uint64_t i_version)
{
    this->pid     = i_pid;
    this->version = i_version;
}

