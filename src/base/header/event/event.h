
#ifndef cardinal_event_message_hpp
#define cardinal_event_message_hpp

#ifndef BPF_PROGRAM_COMPILE
    #include <stdint.h>
    #include <unistd.h>
#endif

#include "event_head.h"
#include "event_body.h"

#define DEFINE_EVENT_DATA(i_name)   \
typedef struct Event##i_name        \
{                                   \
    EventHead               head;   \
    EventBody##i_name       body;   \
} Event##i_name                     \

#pragma pack(push, 8)
    #if defined(OS_LINUX)
    typedef union EventBody
    {
        EventBodyClone              clone;
        EventBodyConnect            connect;
        EventBodyExec               exec;
        EventBodyExitGroup          exit_group;
        EventBodyWClose             wclose;
        EventBodyRClose             rclose;
        EventBodyRecvfrom           recvfrom;
        EventBodyRecvmsg            recvmsg;
        EventBodySkbConsumeUdp      skb_consume_udp;
        EventBodyRename             rename;
        EventBodyRenameat           renameat;
        EventBodyRenameat2          renameat2;
        EventBodyVfsRename          vfs_rename;
        EventBodyUDDS               udds;
    } EventBody;
    #elif defined(OS_MACOS)
    typedef struct EventBody
    {
        EventBodyEndpointSecurityPtr es{nullptr};
    } EventBody;
    #endif

    typedef struct Event
    {
        EventHead head;
        EventBody body;
    } Event;

    DEFINE_EVENT_DATA(Clone);
    DEFINE_EVENT_DATA(Connect);
    DEFINE_EVENT_DATA(Exec);
    DEFINE_EVENT_DATA(ExitGroup);
    DEFINE_EVENT_DATA(WClose);
    DEFINE_EVENT_DATA(RClose);
    DEFINE_EVENT_DATA(Recvfrom);
    DEFINE_EVENT_DATA(Recvmsg);
    DEFINE_EVENT_DATA(SkbConsumeUdp);
    DEFINE_EVENT_DATA(Rename);
    DEFINE_EVENT_DATA(Renameat);
    DEFINE_EVENT_DATA(Renameat2);
    DEFINE_EVENT_DATA(VfsRename);
#pragma pack(pop)

#endif // cardinal_event_message_hpp
