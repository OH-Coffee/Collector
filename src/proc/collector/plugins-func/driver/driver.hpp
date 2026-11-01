
#ifndef plugin_event_source_driver_hpp
#define plugin_event_source_driver_hpp

#include "plugin/plugin.hpp"
#include "plugin/plugin_helper.hpp"

#include "plugins-func/driver/engine/engine_manager.hpp"


class Driver : public Plugin
{
public:
    explicit Driver(message::MessageServer* i_server);
            ~Driver() override;
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
    ret_code msg_handler_for_event_register(uint64_t i_type, uint8_t *i_buf, size_t i_len);
    ret_code msg_handler_for_event_unregister(uint64_t i_type, uint8_t *i_buf, size_t i_len);
    ret_code msg_handler_for_event_unregister_all(uint64_t i_type, uint8_t *i_buf, size_t i_len);
private:
    ret_code msg_handler_for_plugin_pause(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_resume(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code thread_for_disk_access_monitor();
private:
    std::atomic<PluginState>    _plugin_state{};
private:
    EngineManager               _engine_manager;
    EngineManager::Names        _engine_names;
private:
    std::atomic_bool            _is_pause{false};
};

#endif /* plugin_event_source_driver_hpp */

