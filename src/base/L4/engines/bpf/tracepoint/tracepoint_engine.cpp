
#include "header/consts.hpp"

#include "base64/base64.hpp"
#include "system/sys_info.hpp"

#include "engines/bpf/tracepoint/tracepoint_engine.hpp"

static const std::string g_arch                     = sys_machine();
static const std::string g_tracepoint_bpf_prog_dir  = "/tp/";

// 路径相同的BPF程序，buff_name、buff_size只有最先被加载的数据生效

bpf::LoaderArgPtr TracepointEngine::get_loader_args(EventType i_event)
{
    bpf::LoaderArgPtr r_arg = NEW_BPF_LOADER_ARG();

    switch (i_event) {
        case EVENT_TYPE_CLONE: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_proc_base.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_clone",
                .func_name  = "callback_for_enter_clone"
            });
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_exit_clone",
                .func_name  = "callback_for_exit_clone"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 16 * BPF_BUFF_DEFAULT_MEM_SIZE; // 16MB
        } break;
        case EVENT_TYPE_CONNECT: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_connect.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_connect",
                .func_name  = "callback_for_enter_connect"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_EXEC_ENTRY: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_proc_base.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_execve",
                .func_name  = "callback_for_enter_execve"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 16 * BPF_BUFF_DEFAULT_MEM_SIZE; // 16MB
        } break;
        case EVENT_TYPE_EXEC: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_proc_base.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_exit_execve",
                .func_name  = "callback_for_exit_execve"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 16 * BPF_BUFF_DEFAULT_MEM_SIZE; // 16MB
        } break;
        case EVENT_TYPE_EXIT_GROUP: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_proc_base.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_exit_group",
                .func_name  = "callback_for_enter_exit_group"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 16 * BPF_BUFF_DEFAULT_MEM_SIZE; // 16MB
        } break;
        case EVENT_TYPE_RECVMSG: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_recvmsg.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_recvmsg",
                .func_name  = "callback_for_enter_recvmsg"
            });
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_exit_recvmsg",
                .func_name  = "callback_for_exit_recvmsg"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_RECVFROM: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_recvfrom.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_recvfrom",
                .func_name  = "callback_for_enter_recvfrom"
            });
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_exit_recvfrom",
                .func_name  = "callback_for_exit_recvfrom"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_RENAME: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_rename.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_rename",
                .func_name  = "callback_for_enter_rename"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_RENAMEAT: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_rename.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_renameat",
                .func_name  = "callback_for_enter_renameat"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_RENAMEAT2: {
            r_arg->prog_info.path = g_tracepoint_bpf_prog_dir + base64_encode(g_arch + "-" + "tp_rename.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "syscalls",
                .hook_name  = "sys_enter_renameat2",
                .func_name  = "callback_for_enter_renameat2"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        default: { r_arg = nullptr; } break;
    }

    // 需要将内存大小转换为页数
    if(nullptr != r_arg) {
        r_arg->prog_info.buff_size = bpf::change_buffer_size(r_arg->prog_info.buff_size);
    }

    return r_arg;
}
