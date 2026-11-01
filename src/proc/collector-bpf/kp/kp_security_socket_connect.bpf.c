#include "bpf_public_header.h"

NEW_PERCPU_ARRAY(EventConnect, 1, buf_for_connect);

SEC("kprobe/security_socket_connect")
int callback_for_enter_security_socket_connect(struct pt_regs *ctx)
{
    do {
        uint32_t zero = 0;
        EventConnect *data = bpf_map_lookup_elem(&buf_for_connect, &zero);
        if (NULL == data) { break; }

        struct sockaddr *addr = (struct sockaddr*)PT_REGS_PARM2(ctx);
        int addr_len = (int)PT_REGS_PARM3(ctx);

        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        data->head.event_type        = EVENT_TYPE_CONNECT;
        data->head.event_producer    = EP_BPF_KPROBE;
        data->head.event_body_len    = sizeof(data->body);

        data->head.event_process.pid = pid_tgid >> 32;
        data->head.event_process.uid = uid_gid  & 0xffffffff;

        data->body.fd       = 0;
        data->body.addr_len = addr_len;

        if (bpf_probe_read(&data->body.addr, MIN(addr_len, sizeof(data->body.addr)), addr)) {
            break;
        }

        if (data->body.addr.ss_family != AF_INET && data->body.addr.ss_family != AF_INET6) {
            break;
        }

        bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
    } while(false);

    return 0;
}
