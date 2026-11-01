#include "plugin/plugin.hpp"
#include "plugin/plugin_event.hpp"
#include "plugin/plugin_helper.hpp"

#include "plugins-data/net/plugin_net_event_handler_linux/handler__dns.hpp"

namespace cardinal::plugins::plugin_net::handlers {
    ret_code event_handler_for_connect(const EventPtr &i_msg, SCHEMA *o_msg);
    ret_code event_handler_for_recvmsg(const EventPtr &i_msg, SCHEMA *o_msg);
    ret_code event_handler_for_recvfrom(const EventPtr &i_msg, SCHEMA *o_msg);
    ret_code event_handler_for_udds_dns(const EventPtr &i_msg, SCHEMA *o_msg);
    ret_code event_handler_for_skb_consume_udp(const EventPtr &i_msg, SCHEMA *o_msg);
} // namespace cardinal::plugins::plugin_net::handlers

