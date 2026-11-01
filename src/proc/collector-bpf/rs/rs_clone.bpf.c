#include "bpf_public_header.h"

struct syscalls_enter_args {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    uint64_t                clone_flags;
    uint64_t                newsp;
    int *                   parent_tidptr;
    int *                   child_tidptr;
    uint64_t                tls;
    
};

struct syscalls_exit_args {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 ret;
};

FUN_TRACEPOINT_ENTER(clone, struct syscalls_exit_args *i_args)
{
    int r_code = 0;
    
    do {
        if(i_args->ret <= 0) { break; }

        EventClone data = {};
        
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        data.head.event_type       = EVENT_TYPE_CLONE;
        data.head.event_producer   = EP_BPF_TRACEPOINT;

        data.head.event_process.pid = pid_tgid >> 32;
        data.head.event_process.uid = uid_gid  & 0xffffffff;

        data.body.pid_parent = pid_tgid >> 32;
        data.body.pid_child  = i_args->ret;

        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, &data, sizeof(data));
    } while (false);

    return r_code;
}