#ifndef bpf_public_header_h
#define bpf_public_header_h

#include <linux/sched.h>
#include <linux/trace_events.h>
#include <linux/version.h>
#include <linux/kdev_t.h>
#include <linux/rcupdate.h>
#include <uapi/linux/bpf.h>

#include "header/event/event.h"

#include "bpf_helpers.h"
#include "bpf_tracing.h"
#include "bpf_endian.h"


SEC("license") char _license[] = "GPL";
SEC("version") uint32_t _version   = LINUX_VERSION_CODE;

SEC(".maps")
struct {
    __uint(type,        BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size,    sizeof(__u32));
    __uint(value_size,  sizeof(__u32));
} events;


#ifndef MIN
#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#endif

#define IS_SET(i_set, i_flag) ((i_flag) == ((i_set) & (i_flag)))


#define SEC_TRACEPOINT_ENTER(i_name)    SEC("tracepoint/syscalls/sys_enter_" i_name)
#define SEC_TRACEPOINT_EXIT(i_name)     SEC("tracepoint/syscalls/sys_exit_"  i_name)

#define FUN_TRACEPOINT_ENTER(i_name, ...)               \
    SEC_TRACEPOINT_ENTER(#i_name)                       \
    int callback_for_enter_##i_name(__VA_ARGS__)        \

#define FUN_TRACEPOINT_EXIT(i_name,  ...)               \
    SEC_TRACEPOINT_EXIT(#i_name)                        \
    int callback_for_exit_##i_name(__VA_ARGS__)         \


#define NEW_HASH_MAP(i_type, i_num, i_name)             \
    SEC(".maps")                                        \
    struct {                                            \
        __uint(type,        BPF_MAP_TYPE_HASH);         \
        __uint(key_size,    sizeof(uint64_t));          \
        __uint(value_size,  sizeof(i_type));            \
        __uint(max_entries, i_num);                     \
    } i_name                                            \

#define NEW_PERCPU_ARRAY(i_type, i_num, i_name)         \
    SEC(".maps")                                        \
    struct {                                            \
        __uint(type,        BPF_MAP_TYPE_PERCPU_ARRAY); \
        __type(key,         uint32_t);                  \
        __type(value,       i_type);                    \
        __uint(max_entries, i_num);                     \
    } i_name                                            \


#endif // bpf_public_header_h