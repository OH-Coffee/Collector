
#ifndef plugin_net_hpp
#define plugin_net_hpp

#include <time.h>

#include "plugin/plugin.hpp"
#include "plugin/plugin_event.hpp"
#include "plugin/plugin_helper.hpp"

#include "thread/thread_pool.hpp"

#if defined(OS_LINUX)
    #include "engines/event_observer.hpp"
#endif

#if defined(OS_MACOS)
class PluginNet : public Plugin
#elif defined(OS_LINUX)
class PluginNet : public Plugin, EventObserver
#endif
{
public:
    explicit PluginNet(message::MessageServer* i_server);
            ~PluginNet() override;
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
    ret_code msg_handler_for_event_load(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_event_reload(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_event_unload(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_pause(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_resume(PLUGIN_MESSAGE_HANDLER_ARGS);
    
#if defined(OS_MACOS)
private:
    ret_code msg_handler_for_plugin_enable(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_disable(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code msg_handler_for_state_maintenance(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_state_check_stop(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_state_check_start(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code event_handler(SCHEMA &io_msg);
    ret_code event_handler_for_dns(SCHEMA &io_msg);
    ret_code event_handler_for_net(SCHEMA &io_msg);
private:
    ret_code thread_for_event_data_recv();
    ret_code thread_for_event_data_send();
private:
    ret_code socket_listen();
    ret_code socket_loop_recv();
private:
    PluginState state_maintenance(ret_code i_is_enable);
#elif defined(OS_LINUX)
public:
    ret_code event_handler(const EventPtr &i_msg) override;
private:
    ret_code event_handler_run(const EventPtr &i_msg);
#endif


private:
    std::atomic_bool    _plugin_switch{SWITCH_OFF};
private:
    std::atomic_bool         _event_pause{false};
    std::atomic_int64_t      _state_maintenance_stop{0};
    std::atomic<PluginState> _plugin_state{STATE_UNKNOWN};
#if defined(OS_MACOS)
private:
    std::atomic_bool    _event_switch_dns   {SWITCH_OFF};
    std::atomic_bool    _event_switch_net   {SWITCH_OFF};
private:
    UnixSocketServer        _sock_server;
    std::atomic_bool        _sock_server_enable{true};
    std::atomic_uint64_t    _sock_server_listen_counter{0};
private:
    NetExtensionControllerPtr   _ex_controller;
#elif defined(OS_LINUX)
private:
    thread::ThreadPool _threads;
    #ifdef ENABLE_NSS_EXTENSION
    private:
        NssExtensionControllerPtr _ex_controller;
    #endif
#endif
};

#endif /* plugin_net_hpp */

