#include "plugins-data/net/plugin_net_event_handler_linux/handlers.hpp"

static inline ret_code read_udds_dns_event(const EventPtr &i_msg, SCHEMA &o_schema)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if (nullptr == i_msg) { r_code = RC_ARG_ERR; break; }

        const auto *payload = reinterpret_cast<const uint8_t *>(i_msg->body.udds.msg);
        const int64_t payload_len = i_msg->body.udds.msg_len;
        if (payload_len < static_cast<int64_t>(sizeof(SCHEMA_HEAD))) { break; }

        SCHEMA_HEAD head{};
        std::memcpy(&head, payload, sizeof(head));
        SCHEMA schema{};
        schema.Head = head;
        if (RC_OK != schema_check(&schema)) { break; }
        if (SCHEMA_EVENT != head.type) { break; }
        if (EVENT_TYPE_RECVFROM != head.event_type
            && EVENT_TYPE_RECVMSG != head.event_type
            && EVENT_TYPE_SKB_CONSUME_UDP != head.event_type) { break; }
        if (head.data_len < sizeof(EVENT_NET_DNS)
            || head.data_len > sizeof(SCHEMA_BODY)) { break; }
        if (static_cast<uint64_t>(payload_len) < sizeof(SCHEMA_HEAD) + head.data_len) { break; }

        std::memcpy(&schema.Body, payload + sizeof(SCHEMA_HEAD), head.data_len);
        o_schema = schema;
        r_code = RC_OK;
    } while (false);

    return r_code;
}

namespace cardinal::plugins::plugin_net::handlers {

ret_code event_handler_for_udds_dns(const EventPtr &i_msg, SCHEMA *o_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if (i_msg == nullptr || o_msg == nullptr) { r_code = RC_ARG_ERR;     break; }
        SCHEMA cardinal_msg{};
        if (RC_OK != read_udds_dns_event(i_msg, cardinal_msg)) { r_code = RC_UNCONCERNED; break; }

        SCHEMA_HEAD *head = &o_msg->Head;
        SCHEMA_BODY *body = &o_msg->Body;
        std::memset(body, 0, sizeof(*body));

        {
            if (is_host_name(cardinal_msg.Body.dns.query_name)) {
                r_code = RC_UNCONCERNED; break;
            }

            *head = cardinal_msg.Head;
            ::memcpy(body, &cardinal_msg.Body, cardinal_msg.Head.data_len);
        }

        EVENT_NET_DNS *dns_event = &body->dns;
        ProcInfoPtr proc_info_ptr = ProcCache::ins().find(static_cast<pid_t>(head->pid));
        ProcInfoPtr pproc_info_ptr = ProcCache::ins().find(static_cast<pid_t>(head->ppid));

        if (nullptr != proc_info_ptr) {
            string_copy2(proc_info_ptr->path, sizeof(dns_event->path_proc), dns_event->path_proc);
        }
        if (nullptr != pproc_info_ptr) {
            string_copy2(pproc_info_ptr->path, sizeof(dns_event->path_pproc), dns_event->path_pproc);
        }

        r_code = RC_OK;
        EVENT_COUNTER(r_code, head);
    } while (false);

    return r_code;
}

} // namespace cardinal::plugins::plugin_net::handlers {