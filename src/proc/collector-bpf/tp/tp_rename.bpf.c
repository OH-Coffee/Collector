#include "bpf_public_header.h"


NEW_PERCPU_ARRAY(EventRename, 1, buf_for_rename);
NEW_PERCPU_ARRAY(EventRename, 1, buf_for_renameat);
NEW_PERCPU_ARRAY(EventRename, 1, buf_for_renameat2);

static inline bool get_data(EventType i_type, const char *i_oldname, const char *i_newname, EventRename *io_data)
{
    bool r_bool = false;

    do {
        EventHead       *head_ptr = &io_data->head;
        EventBodyRename *body_ptr = &io_data->body;
        
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        head_ptr->event_type       = i_type;
        head_ptr->event_producer   = EP_BPF_TRACEPOINT;
        head_ptr->event_body_len   = sizeof(io_data->body);

        head_ptr->event_process.pid = pid_tgid >> 32;
        head_ptr->event_process.uid = uid_gid  & 0xffffffff;

        body_ptr->flags  =  0;
        body_ptr->olddfd = -1;
        body_ptr->newdfd = -1;

        if(bpf_probe_read(body_ptr->oldname, BUF_LEN_RENAME_PATH, i_oldname)) { break; }
        if(bpf_probe_read(body_ptr->newname, BUF_LEN_RENAME_PATH, i_newname)) { break; }

        r_bool = true;
    } while (false);

    return r_bool;
}


struct syscalls_enter_args_rename {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    const char *            oldname;
    const char *            newname;
};

FUN_TRACEPOINT_ENTER(rename, struct syscalls_enter_args_rename *i_args)
{
    int r_code = 0;
    
    do {
        uint32_t zero = 0;
        EventRename *data = bpf_map_lookup_elem(&buf_for_rename, &zero);
        if (NULL == data) { break; }

        if(get_data(EVENT_TYPE_RENAME, i_args->oldname, i_args->newname, data)) {
            bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
        }
    } while (false);

    return r_code;
}


struct syscalls_enter_args_renameat {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 olddfd;
    const char *            oldname;
    int64_t                 newdfd;
    const char *            newname;
};

FUN_TRACEPOINT_ENTER(renameat, struct syscalls_enter_args_renameat *i_args)
{
    int r_code = 0;
    
    do {
        uint32_t zero = 0;
        EventRename *data = bpf_map_lookup_elem(&buf_for_renameat, &zero);
        if (NULL == data) { break; }

        if(get_data(EVENT_TYPE_RENAMEAT, i_args->oldname, i_args->newname, data)) {
            bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
        }
    } while (false);

    return r_code;
}

struct syscalls_enter_args_renameat2 {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 olddfd;
    const char *            oldname;
    int64_t                 newdfd;
    const char *            newname;
    uint64_t                flags;
};

FUN_TRACEPOINT_ENTER(renameat2, struct syscalls_enter_args_renameat2 *i_args)
{
    int r_code = 0;
    
    do {
        uint32_t zero = 0;
        EventRename *data = bpf_map_lookup_elem(&buf_for_renameat2, &zero);
        if (NULL == data) { break; }
        
        if(get_data(EVENT_TYPE_RENAMEAT2, i_args->oldname, i_args->newname, data)) {
            bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
        }
    } while (false);

    return r_code;
}