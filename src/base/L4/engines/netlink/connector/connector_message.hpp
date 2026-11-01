#ifndef netlink_connector_message_hpp
#define netlink_connector_message_hpp

#include <linux/cn_proc.h>
#include <linux/connector.h>
#include <linux/netlink.h>

typedef enum proc_cn_event ConnectorProcEventType;

typedef struct __attribute__((aligned(NLMSG_ALIGNTO)))
{
    struct nlmsghdr nl_head;

    struct __attribute__((__packed__))
    {
        struct  cn_msg                  cn_msg;
        union {
            enum    proc_cn_mcast_op    cn_mcast;
            struct  proc_event          proc_event;
        } data;

    } nl_body;
} netlink_connector_message;

#endif // netlink_connector_message_hpp