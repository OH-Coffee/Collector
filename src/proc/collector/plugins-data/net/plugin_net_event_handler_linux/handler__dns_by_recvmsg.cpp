#include "plugins-data/net/plugin_net_event_handler_linux/handlers.hpp"

namespace cardinal::plugins::plugin_net::handlers {

ret_code event_handler_for_recvmsg(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR; break; }

        const pid_t proc_pid  = i_msg->head.event_process.pid;

        ProcInfo    proc_info{};
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(proc_pid);
        if(nullptr != proc_info_ptr) {
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
        EVENT_NET_DNS *dns_event = &body->dns;

        // if(RC_OK != (r_code = match__aegis_dlh(head, (uint8_t *)proc_info_ptr->path.c_str()))) { break; }

        {
            const struct sockaddr *addr = &i_msg->body.recvmsg.addr;
            string_copy2(proc_info_ptr->path, sizeof(dns_event->path_proc), dns_event->path_proc);

            int addr_port = 0;
            switch (addr->sa_family) {
                case AF_INET:  { addr_port = ntohs(((struct sockaddr_in  *)addr)->sin_port);  } break;
                case AF_INET6: { addr_port = ntohs(((struct sockaddr_in6 *)addr)->sin6_port); } break;
            }
            if(53 != addr_port) { r_code = RC_UNCONCERNED; break; }

            DnsParseResult dns_info = {};
            if(!parse_dns_protocol((unsigned char *)i_msg->body.recvmsg.data, i_msg->body.recvmsg.data_len, &dns_info)) {
                r_code = RC_UNCONCERNED; break;
            }

            dns_event->query_type    = dns_info.QueryType;
            dns_event->query_result  = dns_info.QueryResult;
            dns_event->query_options = dns_info.QueryOptions;

            string_copy2(dns_info.QueryName, sizeof(dns_event->query_name), dns_event->query_name);
            string_copy2(dns_info.AnswerName, sizeof(dns_event->answers), dns_event->answers);

            if(is_host_name(dns_event->query_name)) { r_code = RC_UNCONCERNED; break; }
        }
        if(RC_UNCONCERNED == r_code) { break; }
        r_code = RC_OK;

        ProcInfoPtr parent_proc = ProcCache::ins().find(proc_info_ptr->ppid);
        if(nullptr != parent_proc) {
            string_copy2(parent_proc->path, sizeof(dns_event->path_pproc), dns_event->path_pproc);
        }

        // if(RC_OK != (r_code = match__aegis_mix(o_msg, proc_info_ptr, parent_proc))) { break; }

        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}

} // namespace cardinal::plugins::plugin_net::handlers