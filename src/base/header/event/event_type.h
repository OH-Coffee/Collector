
#ifndef cardinal_event_type_hpp
#define cardinal_event_type_hpp

#ifdef OS_MACOS
    #import <EndpointSecurity/EndpointSecurity.h>
#endif


typedef enum EventType : int64_t {
    EVENT_TYPE_UNKNOWN = -1,

#if defined(OS_LINUX)
    // 进程
    EVENT_TYPE_FORK             = 1001,
    EVENT_TYPE_CLONE            = 1002,
    EVENT_TYPE_EXEC             = 1003,
    EVENT_TYPE_EXIT_GROUP       = 1004,
    EVENT_TYPE_EXEC_ENTRY       = 1005,
    // 文件
    EVENT_TYPE_OPEN             = 2001,
    EVENT_TYPE_RCLOSE           = 2002,
    EVENT_TYPE_WCLOSE           = 2003,
    EVENT_TYPE_RENAME           = 2004,
    EVENT_TYPE_RENAMEAT         = 2005,
    EVENT_TYPE_RENAMEAT2        = 2006,
    EVENT_TYPE_VFS_RENAME       = 2007,
    // 网络
    EVENT_TYPE_CONNECT          = 3001,
    EVENT_TYPE_RECVFROM         = 3002,
    EVENT_TYPE_RECVMSG          = 3003,
    EVENT_TYPE_SKB_CONSUME_UDP  = 3004,
    // 外部采集器
    EVENT_TYPE_GUDS             = 9999, // GENERAL UNIX DOMAIN SOCKET

    EVENT_TYPE_ALL,
#elif defined(OS_MACOS)
    EVENT_TYPE_FORK             = ES_EVENT_TYPE_NOTIFY_FORK,
    EVENT_TYPE_EXEC             = ES_EVENT_TYPE_NOTIFY_EXEC,
    EVENT_TYPE_EXIT_GROUP       = ES_EVENT_TYPE_NOTIFY_EXIT,
    EVENT_TYPE_OPEN             = ES_EVENT_TYPE_NOTIFY_OPEN,
    EVENT_TYPE_CREATE           = ES_EVENT_TYPE_NOTIFY_CREATE,
    EVENT_TYPE_CLOSE            = ES_EVENT_TYPE_NOTIFY_CLOSE,
    EVENT_TYPE_RENAME           = ES_EVENT_TYPE_NOTIFY_RENAME,
    
    E_TYPE_ALL              = ES_EVENT_TYPE_LAST,
#endif
} EventType;


#endif // cardinal_event_type_hpp
