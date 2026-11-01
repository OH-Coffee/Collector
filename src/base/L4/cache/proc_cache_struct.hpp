
#ifndef proc_cache_struct_h
#define proc_cache_struct_h

#include "header/pch.hpp"

// 缓存项中的进程信息
typedef struct
{
    pid_t           pid;
    pid_t           ppid;

    pid_t           pgid;   // 进程gid，注意和用户gid区别
    uid_t           uid;    // 进程有效用户ID
    pid_t           sid;    // 进程会话id
    uint64_t        flags;  // 进程内核标志
    
    std::string     name;
    std::string     path;
    std::string     args;
    std::string     envs;

    std::string     work_dir;
    std::string     root_dir;
    
    struct timeval  stime;
} ProcInfo;

typedef std::shared_ptr<ProcInfo> ProcInfoPtr;


// 缓存项
typedef struct ProcCacheItem
{
public:
    time_t                          last_time;
    ProcInfoPtr                     last_info;
    std::map<uint64_t, ProcInfoPtr> proc_infos;
public:
    ProcCacheItem();
} ProcCacheItem;

typedef std::shared_ptr<ProcCacheItem> ProcCacheItemPtr;


// 缓存查询键
typedef struct ProcCacheKey
{
public:
    pid_t       pid;
    uint64_t    version;
public:
    ProcCacheKey(pid_t i_pid, uint64_t i_version);
public:
    ProcCacheKey()              : ProcCacheKey(-1, 0)    { }
    ProcCacheKey(pid_t i_pid)   : ProcCacheKey(i_pid, 0) { }
} ProcCacheKey;

#define NEW_PROC_INFO(i_args)           std::make_shared<ProcInfo>(i_args)
#define NEW_PROC_CACHE_ITEM(i_args)     std::make_shared<ProcCacheItem>(i_args)


#endif /* proc_cache_struct_h */
