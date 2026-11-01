{
    .func_name          = "clone",
    .file_name          = "tp_proc_base.bpf.o",
    .enter_hook_name    = "sys_enter_clone",
    .enter_func_name    = "callback_for_enter_clone",
    .exit_hook_name     = "sys_exit_clone",
    .exit_func_name     = "callback_for_exit_clone",
    .perf_buff_name     = "events"
},
{
    .func_name          = "connect",
    .file_name          = "tp_connect.bpf.o",
    .enter_hook_name    = "sys_enter_connect",
    .enter_func_name    = "callback_for_enter_connect",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "security_socket_connect",
    .file_name          = "kp_security_socket_connect.bpf.o",
    .enter_hook_name    = "security_socket_connect",
    .enter_func_name    = "callback_for_enter_security_socket_connect",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "execve",
    .file_name          = "tp_proc_base.bpf.o",
    .enter_hook_name    = "sys_enter_execve",
    .enter_func_name    = "callback_for_enter_execve",
    .exit_hook_name     = "sys_exit_execve",
    .exit_func_name     = "callback_for_exit_execve",
    .perf_buff_name     = "events"
},
{
    .func_name          = "exit_group",
    .file_name          = "tp_proc_base.bpf.o",
    .enter_hook_name    = "sys_enter_exit_group",
    .enter_func_name    = "callback_for_enter_exit_group",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "recvfrom",
    .file_name          = "tp_recvfrom.bpf.o",
    .enter_hook_name    = "sys_enter_recvfrom",
    .enter_func_name    = "callback_for_enter_recvfrom",
    .exit_hook_name     = "sys_exit_recvfrom",
    .exit_func_name     = "callback_for_exit_recvfrom",
    .perf_buff_name     = "events"
},
{
    .func_name          = "recvmsg",
    .file_name          = "tp_recvmsg.bpf.o",
    .enter_hook_name    = "sys_enter_recvmsg",
    .enter_func_name    = "callback_for_enter_recvmsg",
    .exit_hook_name     = "sys_exit_recvmsg",
    .exit_func_name     = "callback_for_exit_recvmsg",
    .perf_buff_name     = "events"
},
{
    .func_name          = "rename",
    .file_name          = "tp_rename.bpf.o",
    .enter_hook_name    = "sys_enter_rename",
    .enter_func_name    = "callback_for_enter_rename",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "renameat",
    .file_name          = "tp_rename.bpf.o",
    .enter_hook_name    = "sys_enter_renameat",
    .enter_func_name    = "callback_for_enter_renameat",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "renameat2",
    .file_name          = "tp_rename.bpf.o",
    .enter_hook_name    = "sys_enter_renameat2",
    .enter_func_name    = "callback_for_enter_renameat2",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "vfs_rename",
    .file_name          = "kp_vfs_rename.bpf.o",
    .enter_hook_name    = "vfs_rename",
    .enter_func_name    = "callback_for_enter_vfs_rename",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "skb_consume_udp_normal_419",
    .file_name          = "kp_skb_consume_udp_normal_419.bpf.o",
    .enter_hook_name    = "skb_consume_udp",
    .enter_func_name    = "callback_for_enter_skb_consume_udp",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "skb_consume_udp_normal_54",
    .file_name          = "kp_skb_consume_udp_normal_54.bpf.o",
    .enter_hook_name    = "skb_consume_udp",
    .enter_func_name    = "callback_for_enter_skb_consume_udp",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "skb_consume_udp_hisi_419",
    .file_name          = "kp_skb_consume_udp_hisi_419.bpf.o",
    .enter_hook_name    = "skb_consume_udp",
    .enter_func_name    = "callback_for_enter_skb_consume_udp",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},
{
    .func_name          = "skb_consume_udp_hisi_54",
    .file_name          = "kp_skb_consume_udp_hisi_54.bpf.o",
    .enter_hook_name    = "skb_consume_udp",
    .enter_func_name    = "callback_for_enter_skb_consume_udp",
    .exit_hook_name     = "",
    .exit_func_name     = "",
    .perf_buff_name     = "events"
},