#ifndef KP_SKB_CONSUME_UDP_INCLUDE_H
#define KP_SKB_CONSUME_UDP_INCLUDE_H

#define KBUILD_MODNAME ""
#include "bpf_public_header.h"
#include <linux/skbuff.h>
#include <linux/string.h>
#include <net/udp.h>

#if BITS_PER_LONG != 64
#error "invalid BITS_PER_LONG"
#endif

//#ifndef __aarch64__
//#error "invalid arch"
//#endif

typedef struct
{
    EventSkbConsumeUdp  data;
    char                buff[BUF_LEN_RECV_DATA];  // 涉及分片重组，需要两倍缓冲区
} EventSkbConsumeUdpPrivate;
NEW_PERCPU_ARRAY(EventSkbConsumeUdpPrivate, 1, buf_for_skb_consume_udp);

// 不要系统的定义，使用cross里面的定义
#undef VMEMMAP_START
#undef PAGE_OFFSET 
#undef PAGE_SIZE

#endif
