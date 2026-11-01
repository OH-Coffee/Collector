#include "bpf_public_header.h"


struct clone_param {
    uint64_t                flags;
    uint64_t                newsp;
    int *                   parent_tidptr;
    int *                   child_tidptr;
    uint64_t                tls;
};
NEW_HASH_MAP(struct clone_param, 1024 * 2, hash_map_for_clone);


struct syscalls_enter_args_clone {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    uint64_t                flags;
    uint64_t                newsp;
    int *                   parent_tidptr;
    int *                   child_tidptr;
    uint64_t                tls;
};

FUN_TRACEPOINT_ENTER(clone, struct syscalls_enter_args_clone *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();

        struct clone_param clone_param = {};
        clone_param.flags             = i_args->flags;
        clone_param.newsp             = i_args->newsp;
        clone_param.parent_tidptr     = i_args->parent_tidptr;
        clone_param.child_tidptr      = i_args->child_tidptr;
        clone_param.tls               = i_args->tls;

        bpf_map_update_elem(&hash_map_for_clone, &pid_tgid, &clone_param, BPF_ANY);
    } while (false);

    return r_code;
}


struct syscalls_exit_args_clone {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 ret;
};

FUN_TRACEPOINT_EXIT(clone, struct syscalls_exit_args_clone *i_args)
{
    int r_code = 0;
    
    do {
        if(i_args->ret <= 0) { break; }

        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        struct clone_param *clone_param_ptr = bpf_map_lookup_elem(&hash_map_for_clone, &pid_tgid);
        if(NULL == clone_param_ptr) { break; }
            uint64_t clone_flags = clone_param_ptr->flags;
            uint64_t clone_newsp = clone_param_ptr->newsp;
        bpf_map_delete_elem(&hash_map_for_clone, &pid_tgid);

        if(IS_SET(clone_flags, CLONE_THREAD)) { break; }

        EventClone data = {};

        data.head.event_type       = EVENT_TYPE_CLONE;
        data.head.event_producer   = EP_BPF_TRACEPOINT;
        data.head.event_body_len   = sizeof(data.body);

        data.head.event_process.pid = pid_tgid >> 32;
        data.head.event_process.uid = uid_gid  & 0xffffffff;

        data.body.pid_parent = pid_tgid >> 32;
        data.body.pid_child  = i_args->ret;

        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, &data, sizeof(data));
    } while (false);

    return r_code;
}

struct syscalls_enter_args_execve {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    const char *            filename;
    const char *const *     argv;
    const char *const *     envp;
};
NEW_PERCPU_ARRAY(EventExec, 1, buf_for_exec);
NEW_HASH_MAP(EventExec, 1024 * 2, hash_map_for_execve);

FUN_TRACEPOINT_ENTER(execve, struct syscalls_enter_args_execve *i_args)
{
    int r_code = 0;
    
    do {
        uint32_t key = 0;
        EventExec *data = bpf_map_lookup_elem(&buf_for_exec, &key);
        if(NULL == data) { break; }
        
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        data->head.event_type       = EVENT_TYPE_EXEC;
        data->head.event_producer   = EP_BPF_TRACEPOINT;
        data->head.event_body_len   = sizeof(data->body);

        data->head.event_process.pid = pid_tgid >> 32;
        data->head.event_process.uid = uid_gid  & 0xffffffff;

        __builtin_memset(&(data->body.argv[0]), 0, sizeof(data->body.argv));

        uint64_t max    = BUF_LEN_EXEC_ARGV / 2;
        uint64_t offset = 0;

        #pragma unroll(32)
        for (int i = 0; i < 19; i++) {
            if (offset >= max) { break; }

            const char *str_ptr = NULL;
            if(0 != bpf_probe_read(&str_ptr, sizeof(str_ptr), &i_args->argv[i]) || str_ptr == NULL) {
                break;
            }

            uint64_t size = max - offset;

            int read_num = bpf_probe_read_str(&data->body.argv[offset], size, str_ptr);
            if(read_num > 0) {
                if((offset += read_num) < max) {
                    data->body.argv[offset - 1] = ' ';
                }
            }
        }
        if(offset > 0 && offset < max) {
            data->body.argv[offset - 1] = '\0';
        }

        bpf_probe_read(data->body.path, BUF_LEN_EXEC_PATH, i_args->filename);

        // 将数据插入到 HASH_MAP 中
        bpf_map_update_elem(&hash_map_for_execve, &pid_tgid, data, BPF_ANY);
        //bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
    } while (false);

    return r_code;
}

struct syscalls_exit_args_execve {
    struct trace_entry      ent;
    int32_t                 syscall_num;

    int64_t                 ret;
};
FUN_TRACEPOINT_EXIT(execve, struct syscalls_exit_args_execve *i_args)
{
    int r_code = 0;
    
    do {
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        EventExec *data = bpf_map_lookup_elem(&hash_map_for_execve, &pid_tgid);
        if(data) {
            bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
            bpf_map_delete_elem(&hash_map_for_execve, &pid_tgid);
            break;
        }

        uint32_t key = 0;
        EventExec *pdata = bpf_map_lookup_elem(&buf_for_exec, &key);
        if(NULL == pdata) { break; }
        pdata->head.event_type       = EVENT_TYPE_EXEC;
        pdata->head.event_producer   = EP_BPF_TRACEPOINT;
        pdata->head.event_body_len   = sizeof(data->body);
        pdata->head.event_process.pid = pid_tgid >> 32;
        pdata->head.event_process.uid = uid_gid  & 0xffffffff;
        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, pdata, sizeof(*pdata));
    } while (false);

    return r_code;
}


struct syscalls_enter_args_exit_group {
    struct trace_entry  ent;
    int32_t                 syscall_num;

    int64_t             error_code;
};

FUN_TRACEPOINT_ENTER(exit_group, struct syscalls_enter_args_exit_group *i_args)
{
    int r_code = 0;
    
    do {
        EventExitGroup data = {};
        
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        data.head.event_type       = EVENT_TYPE_EXIT_GROUP;
        data.head.event_producer   = EP_BPF_TRACEPOINT;
        data.head.event_body_len   = sizeof(data.body);

        data.head.event_process.pid = pid_tgid >> 32;
        data.head.event_process.uid = uid_gid  & 0xffffffff;

        data.body.exit_code   = i_args->error_code;
        data.body.exit_signal = -1;

        bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, &data, sizeof(data));
    } while (false);

    return r_code;
}