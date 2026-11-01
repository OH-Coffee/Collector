#include <arpa/inet.h>
#include <netinet/in.h>

#include "plugins-data/net/plugin_net_event_handler_linux/handlers.hpp"

namespace cardinal::plugins::plugin_net::handlers {

ret_code event_handler_for_connect(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if (i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR; break; }

        const pid_t proc_pid = i_msg->head.event_process.pid;

        ProcInfo    proc_info{};
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(proc_pid);
        if (nullptr != proc_info_ptr) {
            ;
        } else {
            if (try_get_proc_base_info(proc_pid, proc_info)) {
                ProcCache::ins().insert(proc_pid, proc_info_ptr = std::make_shared<ProcInfo>(proc_info));
                LOG_DIAGN("[PluginNet]\t[ YES] 进程：%d 在缓存中未查到该数据 -> 再次获取成功; 错误码：%d", proc_pid, RC_OK);
            } else {
                LOG_DIAGN("[PluginNet]\t[ NO] 进程：%d 再次尝试获取仍然失败 -> 数据即将丢弃; 错误码：%d", proc_pid, RC_CACHE_ERROR); break;
            }
        }

        SCHEMA_HEAD *head = &o_msg->Head;
        SCHEMA_BODY *body = &o_msg->Body;
        
        std::memset(body, 0, sizeof(*body));
        set_header(i_msg->head.event_type, proc_info_ptr, head);

        EVENT_NET_CONNECT *connect_info = &body->connect;
        connect_info->protocol = IPPROTO_TCP;
        connect_info->is_inbound = false;
        string_copy2(proc_info_ptr->path, sizeof(connect_info->path_proc), connect_info->path_proc);

        {
            const auto sock_addr_ptr = reinterpret_cast<struct sockaddr *>(&i_msg->body.connect.addr);

            switch (sock_addr_ptr->sa_family) {
                case AF_INET: {
                    const auto *dst_addr = reinterpret_cast<const struct sockaddr_in *>(sock_addr_ptr);
                    connect_info->is_ipv6 = false;
                    connect_info->dst_port = ntohs(dst_addr->sin_port);
                    if (nullptr == inet_ntop(AF_INET, &dst_addr->sin_addr,
                                             reinterpret_cast<char *>(connect_info->dst_addr),
                                             sizeof(connect_info->dst_addr))) {
                        r_code = RC_FAIL; break;
                    }
                } break;
                case AF_INET6: {
                    const auto *dst_addr = reinterpret_cast<const struct sockaddr_in6 *>(sock_addr_ptr);
                    connect_info->is_ipv6 = true;
                    connect_info->dst_port = ntohs(dst_addr->sin6_port);
                    if (nullptr == inet_ntop(AF_INET6, &dst_addr->sin6_addr,
                                             reinterpret_cast<char *>(connect_info->dst_addr),
                                             sizeof(connect_info->dst_addr))) {
                        r_code = RC_FAIL; break;
                    }
                } break;
                default: { r_code = RC_UNCONCERNED; break; }
            }

            if (RC_FAIL == r_code || RC_UNCONCERNED == r_code) { break; }
            r_code = RC_OK;
        }

        if (connect_info->dst_port <= 0) {
            r_code = RC_UNCONCERNED; break;
        }

        ProcInfoPtr parent_proc = ProcCache::ins().find(proc_info_ptr->ppid);
        if (nullptr != parent_proc) {
            string_copy2(parent_proc->path, sizeof(connect_info->path_pproc), connect_info->path_pproc);
        }

        // if (RC_OK != (r_code = match__aegis_mix(o_msg, proc_info_ptr, parent_proc))) { break; }

        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}

} // namespace cardinal::plugins::plugin_net::handlers
