
#ifndef plugin_proxy_hpp
#define plugin_proxy_hpp

#include "plugin/plugin.hpp"

class Proxy : public Plugin
{
public:
    explicit Proxy(message::MessageServer* i_server);
            ~Proxy() override;
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    PluginState state() override { return _plugin_state; }
public:
    ret_code message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len) override;
private:
    std::atomic<PluginState> _plugin_state{};
};


#endif /* plugin_proxy_hpp */

