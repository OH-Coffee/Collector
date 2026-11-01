
#ifndef plugins_plugin_cache_hpp
#define plugins_plugin_cache_hpp

#include "config/collector_config.hpp"
#include "engines/event_engine.hpp"
#include "engines/event_observer.hpp"
#include "thread/thread_pool.hpp"

#include "plugin/plugin.hpp"
#include "plugin/plugin_helper.hpp"

#include "plugins-data/cache/modules/pid_cache.hpp"

class PluginCache : public Plugin, EventObserver, PidCacheObserver
{
    friend EventEngine;
    friend PluginManager;
public:
    explicit PluginCache(message::MessageServer* i_server);
            ~PluginCache() override;
private:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
private:
    ret_code init_thread_pool();
private:
    ret_code thread_for_pid_cache_clear();
    ret_code thread_for_proc_cache_upload();
private:
    PluginState state() override { return _plugin_state; }
private:
    ret_code message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len) override;
    ret_code msg_handler_for_plugin_pause(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_plugin_resume(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code msg_handler_for_event_reload(PLUGIN_MESSAGE_HANDLER_ARGS);
    ret_code msg_handler_for_event_unload(PLUGIN_MESSAGE_HANDLER_ARGS);
private:
    ret_code event_handler(const EventPtr &) override;
    ret_code event_handler_run(const EventPtr &);
    ret_code event_handler_for_fork(const EventPtr &);
    ret_code event_handler_for_exec(const EventPtr &);
    ret_code event_handler_for_exit(const EventPtr &);
private:
    ret_code when_pid_insert(pid_t) override;
    ret_code when_pid_delete(pid_t) override;
private:
    PidCache _pid_cache;
private:
    std::atomic_bool _is_pause{false};
private:
    thread::ThreadPool          _threads;
    std::atomic_bool            _upload_proc_cache{false};
    std::atomic<PluginState>    _plugin_state{STATE_UNKNOWN};

};

#endif /* plugins_plugin_cache_hpp */

