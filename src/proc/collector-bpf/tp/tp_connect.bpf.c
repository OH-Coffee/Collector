
#include "bpf_public_header.h"

struct syscalls_enter_args_connect {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 fd;
    struct sockaddr *       addr;
    int64_t                 addr_len;
};

FUN_TRACEPOINT_ENTER(connect, struct syscalls_enter_args_connect *i_args)
{
    int r_code = 0;
    
    do {
        EventConnect data = {};

        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        data.head.event_type        = EVENT_TYPE_CONNECT;
        data.head.event_producer    = EP_BPF_TRACEPOINT;
        data.head.event_body_len    = sizeof(data.body);

        data.head.event_process.pid = pid_tgid >> 32;
        data.head.event_process.uid = uid_gid  & 0xffffffff;

        data.body.fd       = i_args->fd;
        data.body.addr_len = i_args->addr_len;
        if (bpf_probe_read(&data.body.addr, sizeof(struct sockaddr), i_args->addr)) {
            break;
        }

        if (data.body.addr.ss_family != AF_INET && data.body.addr.ss_family != AF_INET6) {
            break; 
        }
        
        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, &data, sizeof(data));
    } while (false);

    return r_code;
}