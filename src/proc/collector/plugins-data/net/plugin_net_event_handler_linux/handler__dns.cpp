#include <cstring>
#include "system/sys_info.hpp"

#include "plugins-data/net/plugin_net_event_handler_linux/handler__dns.hpp"

namespace cardinal::plugins::plugin_net::handlers {

bool is_host_name(const uint8_t *i_name)
{
    static const std::string host_name = sys_hostname();

    if (nullptr != i_name && !host_name.empty()) {
        return (0 == strcmp(host_name.c_str(), (const char *) i_name));
    }

    return false;
}

} // namespace cardinal::plugins::plugin_net::handlers