
#ifndef cardinal_event_head_hpp
#define cardinal_event_head_hpp

#include "event_type.h"

#pragma pack(push, 8)
    typedef struct EventProcess
    {
        pid_t pid;
        uid_t uid;
    } EventProcess;

    typedef enum EventProducer {
        EP_UNKNOWN = 0,

        EP_ESF,
        EP_NETLINK_CONNECTOR,
        EP_INOTIFY,
        EP_FANOTIFY,
        EP_BPF_KPROBE,
        EP_BPF_TRACEPOINT,
        EP_BPF_RAWSYSCALL,
        EP_GUDS,

        EP_MAX
    } EventProducer;

    typedef struct EventHead
    {
        EventType       event_type;
        EventProcess    event_process;
        EventProducer   event_producer;
        int64_t         event_body_len;
    } EventHead;
#pragma pack(pop)

#endif // cardinal_event_head_hpp