
#ifndef plugin_file_hpp
#define plugin_file_hpp

#include "engines/event_engine.hpp"
#include "engines/event_observer.hpp"
#include "thread/thread_pool.hpp"

#include "plugin/plugin.hpp"
#include "plugin/plugin_helper.hpp"

class PluginFile : public Plugin, EventObserver
{
    friend EventEngine;
    friend PluginManager;
public:
    explicit PluginFile(message::MessageServer* i_server);
            ~PluginFile() override;
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    PluginState state() override { return _plugin_state; }
private:
    ret_code message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len) override;
    ret_code msg_handler_for_event_reload(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_event_unload(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_pause(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_resume(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code event_handler(const EventPtr &i_msg) override;
    ret_code event_handler_run(const EventPtr &i_msg);
private:
    thread::ThreadPool          _threads;
    std::atomic_uint64_t        _op_code{0};
    std::atomic<PluginState>    _plugin_state{STATE_UNKNOWN};
private:
    std::atomic_bool _is_pause{false};
};

#endif /* plugin_file_hpp */

