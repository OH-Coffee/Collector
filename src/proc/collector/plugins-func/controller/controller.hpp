#ifndef controller_hpp
#define controller_hpp

#include <list>

#include "plugin/plugin.hpp"
#include "plugin/plugin_helper.hpp"

#include "json/json.h"
#include "logger/logger.hpp"
#include "message/message_client.hpp"
#include "message/message_server.hpp"

class Controller : public PluginManager
{
public:
    Controller();
    ~Controller() override;
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    PluginState state() override { return STATE_READY; }
private:
    ret_code init_plugins();
    
public:
    ret_code message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len) override;
private:
    ret_code msg_handler_for_controller_stop(PLUGIN_MESSAGE_HANDLER_ARGS);    
private:
    std::atomic<PluginState> _plugin_state{};
private:
    std::list< Plugin* >    _plugins;
    message::MessageServer  _message_center;
};


#endif /* controller_hpp */
