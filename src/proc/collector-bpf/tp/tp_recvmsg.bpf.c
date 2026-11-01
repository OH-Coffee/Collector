
#include <linux/in.h>
#include <linux/in6.h>

#include "bpf_public_header.h"

struct recv_args {
    int64_t             fd;
    void *              msg_name;
    int64_t             msg_namelen;
    struct iovec *      iov;
    int64_t             iovlen;
};
NEW_HASH_MAP(struct recv_args, 1024 * 2, hash_map_for_recvmsg);


struct syscalls_enter_args_recvmsg {
    struct trace_entry      ent;
    int32_t                 __syscall_nr;

    int64_t                 fd;
    struct user_msghdr *    msg;
    uint64_t                flags;
};

FUN_TRACEPOINT_ENTER(recvmsg, struct syscalls_enter_args_recvmsg *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();

        struct user_msghdr msg_header = {};
        if (bpf_probe_read_user(&msg_header, sizeof(msg_header), i_args->msg)) {
            break;
        }

        struct recv_args recv_info = {};
        recv_info.fd            = i_args->fd;
        recv_info.msg_name      = msg_header.msg_name;
        recv_info.msg_namelen   = msg_header.msg_namelen;
        recv_info.iov           = msg_header.msg_iov;
        recv_info.iovlen        = msg_header.msg_iovlen;

        if (NULL == recv_info.msg_name || NULL == recv_info.iov || 0 == recv_info.iovlen) {
            break;
        } else {
            bpf_map_update_elem(&hash_map_for_recvmsg, &pid_tgid, &recv_info, BPF_ANY);
        }
    } while (false);

    return r_code;
}


struct syscalls_exit_args_recvmsg {
    struct trace_entry      ent;
    int32_t                 __syscall_nr;
    int64_t                 ret;
};
NEW_PERCPU_ARRAY(EventRecvmsg, 1, buf_for_recvmsg);

FUN_TRACEPOINT_EXIT(recvmsg, struct syscalls_exit_args_recvmsg *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        struct recv_args *recv_info = bpf_map_lookup_elem(&hash_map_for_recvmsg, &pid_tgid);
        if (NULL == recv_info) { break; }
            int64_t         fd          = recv_info->fd;
            char *          msg_name    = recv_info->msg_name;
            int64_t         msg_namelen = recv_info->msg_namelen;
            struct iovec *  iov         = recv_info->iov;
            int64_t         iovlen      = recv_info->iovlen;
        bpf_map_delete_elem(&hash_map_for_recvmsg, &pid_tgid);

        long recvmsg_return_value = i_args->ret;
        if(recvmsg_return_value <= 0 || recvmsg_return_value > BUF_LEN_RECV_DATA) {
            break; // recvmsg()调用失败，或数据大于缓冲区大小，均不处理
        }

        uint32_t zero = 0;
        EventRecvmsg *data = bpf_map_lookup_elem(&buf_for_recvmsg, &zero);
        if (NULL == data) { break; }

        data->head.event_type       = EVENT_TYPE_RECVMSG;
        data->head.event_producer   = EP_BPF_TRACEPOINT;
        data->head.event_body_len   = sizeof(data->body);

        data->head.event_process.pid = pid_tgid >> 32;
        data->head.event_process.uid = uid_gid  & 0xffffffff;

        data->body.fd       = fd;
        data->body.addr_len = sizeof(struct sockaddr);
        if(bpf_probe_read(&data->body.addr, data->body.addr_len, msg_name)) {
            break;
        }

        int64_t          addr_port = 0;
        struct sockaddr *sock_addr = (struct sockaddr *)&data->body.addr;
        switch (sock_addr->sa_family) {
            case AF_INET:  { addr_port = __bpf_ntohs(((struct sockaddr_in  *)sock_addr)->sin_port);  } break;
            case AF_INET6: { addr_port = __bpf_ntohs(((struct sockaddr_in6 *)sock_addr)->sin6_port); } break;
        }
        if(53 != addr_port) { break; }

        struct iovec cur_iov;
        if (bpf_probe_read_user(&cur_iov, sizeof(cur_iov), iov)) { break; }
        if (cur_iov.iov_len < recvmsg_return_value)              { break; }

        data->body.data_len = MIN(recvmsg_return_value, BUF_LEN_RECV_DATA);
        if (bpf_probe_read_user(data->body.data, data->body.data_len, cur_iov.iov_base)) { 
            break;
        }

        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
    } while (false);

    return r_code;
}