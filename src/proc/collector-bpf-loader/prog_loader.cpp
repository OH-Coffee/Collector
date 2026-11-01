#include <list>
#include <string>

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>


#include <arpa/inet.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/epoll.h>

#include "file/file_api.hpp"
#include "process/proc_info.hpp"
#include "string/string_api.hpp"

#include "dns_printer.h"

#define MAP_PAGE_SIZE   1024

// 某些系统上可能无此枚举类型，如果发现类型重定义，将此处定义注释即可
// enum bpf_link_type {
//     BPF_LINK_TYPE_UNSPEC            = 0,
//     BPF_LINK_TYPE_RAW_TRACEPOINT    = 1,
//     BPF_LINK_TYPE_TRACING           = 2,
//     BPF_LINK_TYPE_CGROUP            = 3,
//     BPF_LINK_TYPE_ITER              = 4,
//     BPF_LINK_TYPE_NETNS             = 5,
//     BPF_LINK_TYPE_XDP               = 6,
//     BPF_LINK_TYPE_PERF_EVENT        = 7,
//     BPF_LINK_TYPE_KPROBE_MULTI      = 8,
//     BPF_LINK_TYPE_STRUCT_OPS        = 9,
//     BPF_LINK_TYPE_NETFILTER         = 10,
//     BPF_LINK_TYPE_TCX               = 11,
//     BPF_LINK_TYPE_UPROBE_MULTI      = 12,
//     BPF_LINK_TYPE_NETKIT            = 13,
//     BPF_LINK_TYPE_SOCKMAP           = 14,
//     __MAX_BPF_LINK_TYPE,
// };

#include "event.h"
#include "libbpf.h"

static void coll_callback(void *i_args, int cpu, void *i_data, __u32 i_data_size)
{
    Event        *data = (Event *)i_data;
    EventHead    *head = &data->head;
    EventBody    *body = &data->body;


    switch (head->event_type) {
        case EVENT_TYPE_CLONE: {
            printf("typde:%4ld, pid:%8d, uid:%8d, parent_tid:%8d, child_tid:%8d\n",
                    head->event_type,
                    head->event_process.pid,
                    head->event_process.uid,
                    body->clone.pid_parent,
                    body->clone.pid_child
            );
        } break;
        case EVENT_TYPE_CONNECT: {
            struct sockaddr *addr = (struct sockaddr *)&body->connect.addr;

            char addr_ip[128];
            int  addr_port    = 0;

            bzero(addr_ip, sizeof(addr_ip));

            switch (body->connect.addr.ss_family) {
                case AF_INET:  {
                    addr_port = ntohs(((struct sockaddr_in  *)addr)->sin_port);
                    inet_ntop(addr->sa_family, &((struct sockaddr_in  *)addr)->sin_addr,  addr_ip, sizeof(addr_ip));
                } break;
                case AF_INET6: {
                    addr_port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
                    inet_ntop(addr->sa_family, &((struct sockaddr_in6 *)addr)->sin6_addr, addr_ip, sizeof(addr_ip));
                } break;
            }

            // if(addr->sa_family != AF_INET && addr->sa_family != AF_INET6) { break; }
            printf("type:%4ld, pid:%8d, fd:%ld, family:%d, ip:%s, port:%d, addr_len:%ld\n",
                    head->event_type,
                    head->event_process.pid,
                    body->connect.fd,
                    addr->sa_family,
                    addr_ip,
                    addr_port,
                    body->connect.addr_len
            );
        } break;
        case EVENT_TYPE_EXEC_ENTRY: {
            printf("typde:%4ld, pid:%8d, uid:%8d, path:%s, argv:%s\n",
                    head->event_type,
                    head->event_process.pid,
                    head->event_process.uid,
                    body->exec.path,
                    body->exec.argv
            );
        } break;
        case EVENT_TYPE_EXIT_GROUP: {
            printf("typde:%4ld, pid:%8d, eixt_code:%ld\n",
                    head->event_type,
                    head->event_process.pid,
                    body->exit_group.exit_code
            );
        } break;
        case EVENT_TYPE_RECVFROM: {
            struct sockaddr_in *addr = (struct sockaddr_in *)&body->recvfrom.addr;

            char addr_ip[128] = { 0 };
            int  addr_port    = ntohs(addr->sin_port);
            inet_ntop(AF_INET, &addr->sin_addr, addr_ip, sizeof(addr_ip));

            printf("type:%4ld, pid:%d, fd:%ld, ip:%s, port:%d,  addr_len:%ld, data:%s, data_len:%ld\n",
                    head->event_type,
                    head->event_process.pid,
                    body->recvfrom.fd,
                    addr_ip,
                    addr_port,
                    body->recvfrom.addr_len,
                    "",
                    body->recvfrom.data_len
            );
        } break;
        case EVENT_TYPE_RECVMSG: {
            struct sockaddr_in *addr = (struct sockaddr_in *)&body->recvmsg.addr;

            char addr_ip[128] = { 0 };
            int  addr_port    = ntohs(addr->sin_port);
            inet_ntop(AF_INET, &addr->sin_addr, addr_ip, sizeof(addr_ip));

            printf("type:%4ld, pid:%d, fd:%ld, ip:%s, port:%d,  addr_len:%ld, data:%s, data_len:%ld\n",
                    head->event_type,
                    head->event_process.pid,
                    body->recvmsg.fd,
                    addr_ip,
                    addr_port,
                    body->recvmsg.addr_len,
                    "",
                    body->recvmsg.data_len
            );
        } break;
        case EVENT_TYPE_SKB_CONSUME_UDP: {
            printf("type:%4ld, pid:%d, data:%s, reserve2:%d, data_len:%ld\n",
                    head->event_type,
                    head->event_process.pid,
                    "",
                    body->skb_consume_udp.reserve2,
                    body->skb_consume_udp.data_len
            );
            print_dns_resopens((unsigned char *)body->skb_consume_udp.data);
        } break;
        case EVENT_TYPE_RENAME:
        case EVENT_TYPE_RENAMEAT:
        case EVENT_TYPE_RENAMEAT2: {
            printf("typde:%4ld, pid:%8d, %s -> %s\n",
                    head->event_type,
                    head->event_process.pid,
                    body->rename.oldname,
                    body->rename.newname
            );
        } break;
        case EVENT_TYPE_VFS_RENAME: {
            printf("typde:%4ld, pid:%8d, [%lu, %lu, %lu] %s -> %s\n",
                    head->event_type,
                    head->event_process.pid,
                    body->vfs_rename.mount.dev_maj,
                    body->vfs_rename.mount.dev_min,
                    body->vfs_rename.mount.inode,
                    body->vfs_rename.oldname,
                    body->vfs_rename.newname
            );
        } break;
        default: break;
    }
}

static void lost_callback(void *ctx, int cpu, __u64 lost_cnt)
{
    printf("Lost %llu events on CPU #%d!\n", lost_cnt, cpu);
}

#define MAP_PAGE_SIZE 1024

struct bpf_load_args
{
    std::string func_name;
    std::string file_name;

    std::string enter_hook_name;
    std::string enter_func_name;

    std::string exit_hook_name;
    std::string exit_func_name;
    std::string perf_buff_name;
    int         perf_buff_page_size = MAP_PAGE_SIZE;
};

static std::list<struct bpf_load_args> g_bpf_load_args = {
    #include "prog_args.hpp"
};

static bool g_is_exit = false;

static std::list<struct bpf_object  *> g_bpf_objs;
static std::list<struct bpf_link    *> g_bpf_links;

static std::list<int>                  g_perf_buf_fds;
static std::list<struct perf_buffer *> g_perf_bufs;

static void sig_handler(int i_sig)
{
    g_is_exit = true;

    sleep(1);

    for(auto it : g_bpf_objs)  { bpf_object__close(it); }
    for(auto it : g_bpf_links) { bpf_link__destroy(it); }
    for(auto it : g_perf_bufs) { perf_buffer__free(it); }

    for(auto it : g_perf_buf_fds) { close(it); }
}

static int          g_ebpf_type = 0;
static std::string  g_bpf_prog_dir  = "../output/programs";

int main(int argc, char *argv[])
{
    if(argc > 1) {
        g_ebpf_type = atoi(argv[1]);
    }

    std::string func_name = (argc >= 3) ? argv[2] : "";

    struct rlimit lim = {
        .rlim_cur = RLIM_INFINITY,
        .rlim_max = RLIM_INFINITY,
    }; setrlimit(RLIMIT_MEMLOCK, &lim);

    signal(SIGABRT, sig_handler);
    signal(SIGTERM, sig_handler);

    g_bpf_prog_dir = file_dir(proc_path(getpid())) + "/BPF";

    printf(">>> BPF prog dir: %s\n", g_bpf_prog_dir.c_str());

    for(const auto &bpf_arg : g_bpf_load_args) {
        std::string bpf_file_path;

        switch (g_ebpf_type) {
            case 0: { bpf_file_path = g_bpf_prog_dir + "/rs/" + bpf_arg.file_name; } break;
            case 1: { bpf_file_path = g_bpf_prog_dir + "/tp/" + bpf_arg.file_name; } break;
            case 2: { bpf_file_path = g_bpf_prog_dir + "/kp/" + bpf_arg.file_name; } break;
            default: return -1;
        }
        
        if(!func_name.empty() && bpf_arg.func_name != func_name) {
            continue;
        }

        printf(">>> BPF prog file: %s\n", bpf_file_path.c_str());

        bool is_ok = false;

        struct bpf_object   *bpf_obj        = nullptr;
        struct bpf_link     *bpf_link_enter = nullptr;
        struct bpf_link     *bpf_link_exit  = nullptr;
        struct perf_buffer  *bpf_buff       = nullptr;
        int                  bpf_buff_fd    = -1;

        do {
            bpf_obj = bpf_object__open(bpf_file_path.c_str());
            if(libbpf_get_error(bpf_obj)) { break; }

            if(bpf_object__load(bpf_obj)) { break; }

            struct bpf_program *bpf_prog_enter = nullptr;
            struct bpf_program *bpf_prog_exit  = nullptr;

            if(!bpf_arg.enter_func_name.empty()) {
                bpf_prog_enter = bpf_object__find_program_by_name(bpf_obj, bpf_arg.enter_func_name.c_str());
            }
            if(!bpf_arg.exit_func_name.empty()) {
                bpf_prog_exit  = bpf_object__find_program_by_name(bpf_obj, bpf_arg.exit_func_name.c_str());
            }

            if(nullptr != bpf_prog_enter) {
                switch (g_ebpf_type) {
                    case 0:
                    case 1: { bpf_link_enter = bpf_program__attach_tracepoint(bpf_prog_enter, "syscalls", bpf_arg.enter_hook_name.c_str()); } break;
                    case 2: { bpf_link_enter = bpf_program__attach_kprobe(bpf_prog_enter, false, bpf_arg.enter_hook_name.c_str());          } break;
                }
                if (libbpf_get_error(bpf_link_enter)) { break; }
            }

            if(nullptr != bpf_prog_exit) {
                switch (g_ebpf_type) {
                    case 0:
                    case 1: { bpf_link_exit = bpf_program__attach_tracepoint(bpf_prog_exit, "syscalls", bpf_arg.exit_hook_name.c_str()); } break;
                    case 2: { bpf_link_exit = bpf_program__attach_kprobe(bpf_prog_exit, true, bpf_arg.exit_hook_name.c_str());           } break;
                }
                if (libbpf_get_error(bpf_link_exit)) { break; }
            }

            int bpf_map_fd = bpf_object__find_map_fd_by_name(bpf_obj, bpf_arg.perf_buff_name.c_str());
            if(bpf_map_fd < 0) {
                break;
            }

            bpf_buff = perf_buffer__new(bpf_map_fd, bpf_arg.perf_buff_page_size, coll_callback, lost_callback, nullptr, nullptr);
            if (libbpf_get_error(bpf_buff)) {
                break;
            }

            bpf_buff_fd = perf_buffer__epoll_fd(bpf_buff);
            if(bpf_buff_fd < 0) {
                break;
            }

            is_ok = true;
        } while (false);

        if(false == is_ok) {
            if(bpf_buff_fd > 0)             { close(bpf_buff_fd);                   bpf_buff_fd     = 0;        }
            if(nullptr != bpf_obj)          { bpf_object__close(bpf_obj);           bpf_obj         = nullptr;  }
            if(nullptr != bpf_link_enter)   { bpf_link__destroy(bpf_link_enter);    bpf_link_enter  = nullptr;  }
            if(nullptr != bpf_link_exit)    { bpf_link__destroy(bpf_link_exit);     bpf_link_exit   = nullptr;  }
            if(nullptr != bpf_buff)         { perf_buffer__free(bpf_buff);          bpf_buff        = nullptr;  }
            break;
        } else {
            g_bpf_objs.push_back(bpf_obj);
            if(nullptr != bpf_link_enter)  {
                g_bpf_links.push_back(bpf_link_enter);
            }
            if(nullptr != bpf_link_exit)  {
                g_bpf_links.push_back(bpf_link_exit);
            }
            g_perf_bufs.push_back(bpf_buff);
            g_perf_buf_fds.push_back(bpf_buff_fd);
        }
    }

    int epoll_fd = epoll_create1(0);

    for (auto fd : g_perf_buf_fds) {
        struct epoll_event epoll_ev;
        epoll_ev.events  = EPOLLIN;
        epoll_ev.data.fd = fd;

        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &epoll_ev);
    }

    while(!g_is_exit) {
        struct epoll_event epoll_events[10];
        bzero(epoll_events, sizeof(epoll_events));

        int event_num = epoll_wait(epoll_fd, epoll_events, 10, -1);
        if(event_num < 0) { break; }

        for(auto buf : g_perf_bufs) {
            perf_buffer__consume(buf);
        }
    }


    return 0;
}
