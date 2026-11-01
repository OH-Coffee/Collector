
#include <linux/in.h>
#include <linux/in6.h>

#include "bpf_public_header.h"

struct recv_args {
    int64_t             fd;
    void *              buf;
    int64_t             buf_len;
    struct sockaddr *   addr;
    int *               addr_len;
};
NEW_HASH_MAP(struct recv_args, 1024 * 2, hash_map_for_recvfrom);


struct syscalls_enter_args_recvfrom {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 fd;
    void *                  ubuf;
    int64_t                 ubuf_len;
    int64_t                 flags;
    struct sockaddr *       addr;
    int *                   addr_len;
};

FUN_TRACEPOINT_ENTER(recvfrom, struct syscalls_enter_args_recvfrom *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();

        struct recv_args recv_info = {};
        recv_info.fd        = i_args->fd;
        recv_info.buf       = i_args->ubuf;
        recv_info.buf_len   = i_args->ubuf_len;
        recv_info.addr      = i_args->addr;
        recv_info.addr_len  = i_args->addr_len;

        if (NULL == recv_info.buf || 0 == recv_info.buf_len || NULL == recv_info.addr || NULL == recv_info.addr_len) {
            break;
        } else {
            bpf_map_update_elem(&hash_map_for_recvfrom, &pid_tgid, &recv_info, BPF_ANY);
        }
    } while (false);

    return r_code;
}


struct syscalls_exit_args_recvfrom {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 ret;
};
NEW_PERCPU_ARRAY(EventRecvfrom, 1, buf_for_recvfrom);

FUN_TRACEPOINT_EXIT(recvfrom, struct syscalls_exit_args_recvfrom *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        struct recv_args *recv_info = bpf_map_lookup_elem(&hash_map_for_recvfrom, &pid_tgid);
        if (NULL == recv_info) { break; }
            int64_t             fd          = recv_info->fd;
            void *              buf         = recv_info->buf;
            uint64_t            buf_len     = recv_info->buf_len;
            struct sockaddr *   addr        = recv_info->addr;
            uint32_t *          addr_len    = recv_info->addr_len;
        bpf_map_delete_elem(&hash_map_for_recvfrom, &pid_tgid);

        long recvfrom_return_value = i_args->ret;
        if(recvfrom_return_value <= 0 || recvfrom_return_value > BUF_LEN_RECV_DATA) {
            break; // recvfrom()调用失败，或数据大于缓冲区大小，均不处理
        }

        uint32_t zero = 0;
        EventRecvfrom *data = bpf_map_lookup_elem(&buf_for_recvfrom, &zero);
        if (NULL == data) { break; }

        data->head.event_type       = EVENT_TYPE_RECVFROM;
        data->head.event_producer   = EP_BPF_TRACEPOINT;
        data->head.event_body_len   = sizeof(data->body);

        data->head.event_process.pid = pid_tgid >> 32;
        data->head.event_process.uid = uid_gid  & 0xffffffff;

        data->body.fd = fd;

        if(bpf_probe_read(&data->body.addr,     sizeof(struct sockaddr), addr))     { break; }
        if(bpf_probe_read(&data->body.addr_len, sizeof(int),             addr_len)) { break; }

        int64_t          addr_port = 0;
        struct sockaddr *sock_addr = (struct sockaddr *)&data->body.addr;
        switch (sock_addr->sa_family) {
            case AF_INET:  { addr_port = __bpf_ntohs(((struct sockaddr_in  *)sock_addr)->sin_port);  } break;
            case AF_INET6: { addr_port = __bpf_ntohs(((struct sockaddr_in6 *)sock_addr)->sin6_port); } break;
        }
        if(53 != addr_port) { break; }
        
        data->body.data_len = recvfrom_return_value;
        if(bpf_probe_read(&data->body.data, data->body.data_len, buf)) { break; }

        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
    } while (false);

    return r_code;
}