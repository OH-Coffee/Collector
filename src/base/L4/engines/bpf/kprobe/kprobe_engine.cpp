
#include "header/consts.hpp"

#include "logger/logger.hpp"
#include "file/file_api.hpp"
#include "base64/base64.hpp"
#include "system/sys_info.hpp"
#include "process/proc_info.hpp"

#include "engines/bpf/kprobe/kprobe_engine.hpp"

// static const std::string g_arch     = sys_machine();
static const std::string g_arch     = "";
static const std::string g_bpf_dir  = file_dir(proc_path(getpid())) + "/BPF/kp/";

static void get_skb_consume_udp_args(bpf::LoaderArgPtr& o_arg)
{
    // 只支持arm64，x86直接返回
#ifndef __aarch64__
    o_arg = nullptr; return;
#endif

    // 页大小必须是4K，部分arm64系统比如centos7使用64K的页，这里不支持
    {
        int64_t page_size = sys_page_size();
        if (0x1000 != page_size) {
            o_arg = nullptr; return;
        }
    }

    enum {
        KERN_VER_GREATER_THAN_54 = 0x1, // 内核版本号 大于等于5.4小于5.11
        KERN_IS_HISI = 0x2,             // 是海思麒麟的内核
    };
    int flag = 0;

    // 目前uos只有4.19.x 5.4.x 5.10.x的内核，更高的就不支持了
    {
        std::string kern_release = sys_kern_release();
        LOG_INFO("[Driver]\t check kern release：%s", kern_release.c_str());
        if (0 == strncmp(kern_release.c_str(), "4.19.", 5)) {
            // nothing
        } else if (0 == strncmp(kern_release.c_str(), "5.4.", 4)) {
            flag |= KERN_VER_GREATER_THAN_54;
        } else if (0 == strncmp(kern_release.c_str(), "5.10.", 5)) {
            flag |= KERN_VER_GREATER_THAN_54;
        } else {
            o_arg = nullptr; return;
        }
    }

    // 判断是不是华为海思, 不知道有没有更好的办法
    {
        if (0 > ::access("/etc/hw_version", F_OK)) {
            // nothing
        } else {
            flag |= KERN_IS_HISI;
        }
    }

    switch (flag) {
        case 0:
            o_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_skb_consume_udp_normal_419.bpf.o");
            break;
        case KERN_VER_GREATER_THAN_54:
            o_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_skb_consume_udp_normal_54.bpf.o");
            break;
        case KERN_IS_HISI:
            o_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_skb_consume_udp_hisi_419.bpf.o");
            break;
        case (KERN_VER_GREATER_THAN_54 | KERN_IS_HISI):
            o_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_skb_consume_udp_hisi_54.bpf.o");
            break;
        default:
            o_arg = nullptr; return;
    }

    o_arg->link_args.push_back({
        .hook_type  = "kprobe",
        .hook_name  = "skb_consume_udp",
        .func_name  = "callback_for_enter_skb_consume_udp"
    });
    o_arg->prog_info.buff_name = "events";
    o_arg->prog_info.buff_size = 16 * BPF_BUFF_DEFAULT_MEM_SIZE; // 16MB

    return;
}

bpf::LoaderArgPtr KprobeEngine::get_loader_args(EventType i_event)
{
    bpf::LoaderArgPtr r_arg = NEW_BPF_LOADER_ARG();

    switch (i_event) {
        case EVENT_TYPE_CONNECT: {
            r_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_security_socket_connect.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "kprobe",
                .hook_name  = "security_socket_connect",
                .func_name  = "callback_for_enter_security_socket_connect"
            });
            r_arg->prog_info.buff_name = "events";
            r_arg->prog_info.buff_size = 8 * BPF_BUFF_DEFAULT_MEM_SIZE; // 8MB
        } break;
        case EVENT_TYPE_SKB_CONSUME_UDP: {
            get_skb_consume_udp_args(r_arg);
        } break;
        case EVENT_TYPE_VFS_RENAME: {
            r_arg->prog_info.path = g_bpf_dir + (g_arch + "kp_vfs_rename.bpf.o");
            r_arg->link_args.push_back({
                .hook_type  = "kprobe",
                .hook_name  = "vfs_rename",
                .func_name  = "callback_for_enter_vfs_rename"
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

