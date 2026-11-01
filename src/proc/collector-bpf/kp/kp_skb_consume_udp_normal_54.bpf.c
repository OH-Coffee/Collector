#include "kp_skb_consume_udp_include.h"

#include "cross/kmap_normal_54.h"
#include "cross/skb_offset_54.h"
SEC("kprobe/skb_consume_udp")
int callback_for_enter_skb_consume_udp(struct pt_regs *ctx) 
#include "inl/kp_skb_consume_udp.inl"
#include "cross/skb_offset_54.h"
#include "cross/kmap_normal_54.h"
