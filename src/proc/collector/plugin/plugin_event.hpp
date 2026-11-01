
#ifndef plugin_event_hpp
#define plugin_event_hpp

#include <cstdint>

#include "header/pch.hpp"
#include "message/message.hpp"
#include "engines/event_observer.hpp"

typedef enum : uint64_t {
    DATA_MSG_UNKNOWN                = 10000000,
        DATA_MSG_SEND_TO_PROXY,
        DATA_MSG_SEND_TO_WRAPPER,
    DATA_MSG_MAXIMUM                = 19999999,
    
    
    CTRL_MSG_UNKNOWN                = 20000000,
        CTRL_MSG_SEND_TO_SERVER                 = 20010001,
        CTRL_MSG_RECV_FROM_SERVER               = 20010002,
        
        CTRL_MSG_CONTROLLER_STOP                = 20020001,
        CTRL_MSG_CONTROLLER_STATE_UPLOAD        = 20020002,
        CTRL_MSG_CONTROLLER_EXIT_BY_SIGNAL      = 20020003,
        
        CTRL_MSG_CONF_RELOAD                    = 20030001,
        CTRL_MSG_CONF_UNLOAD                    = 20030002,
        CTRL_MSG_RULE_RELOAD                    = 20030003,
        CTRL_MSG_RULE_UNLOAD                    = 20030004,
    
        CTRL_MSG_WRAPPER_UNLOAD                 = 20040001,
    
        CTRL_MSG_PROXY_CONNTECT_OK              = 20050001,
        CTRL_MSG_PROXY_CONNTECT_ERR             = 20050002,
        CTRL_MSG_PROXY_RECONNTECT               = 20050003,
        CTRL_MSG_PROXY_DISCONNTECT_ONCE         = 20050004,
        
        CTRL_MSG_NET_EXTENSION_REGIST           = 20060001,
        CTRL_MSG_NET_EXTENSION_UNREGIST         = 20060002,
        CTRL_MSG_NET_EXTENSION_LOAD             = 20060003,
        CTRL_MSG_NET_EXTENSION_UNLOAD           = 20060004,
        CTRL_MSG_NET_EXTENSION_ENABLE           = 20060005,
        CTRL_MSG_NET_EXTENSION_DISABLE          = 20060006,
        CTRL_MSG_NET_PLUGIN_ENABLE              = 20060010,
        CTRL_MSG_NET_PLUGIN_DISABLE             = 20060011,
        CTRL_MSG_NET_EXTENSION_CONNECT_OK       = 20060014,
        CTRL_MSG_NET_EXTENSION_CONNECT_ERR      = 20060015,
        CTRL_MSG_NET_PLUGIN_STATE_CHECK_STOP    = 20060016,
        CTRL_MSG_NET_PLUGIN_STATE_CHECK_START   = 20060017,
        
        CTRL_MSG_DNS_EXTENSION_REGIST           = 20070001,
        CTRL_MSG_DNS_EXTENSION_UNREGIST         = 20070002,
        CTRL_MSG_DNS_EXTENSION_LOAD             = 20070003,
        CTRL_MSG_DNS_EXTENSION_UNLOAD           = 20070004,
        CTRL_MSG_DNS_EXTENSION_ENABLE           = 20070005,
        CTRL_MSG_DNS_EXTENSION_DISABLE          = 20070006,
        CTRL_MSG_DNS_PLUGIN_ENABLE              = 20070010,
        CTRL_MSG_DNS_PLUGIN_DISABLE             = 20070011,
    
        CTRL_MSG_ES_ALL_REGIST                  = 20080101,
        CTRL_MSG_ES_ALL_UNREGIST                = 20080102,
        CTRL_MSG_ES_ALL_RESUME                  = 20080103,
        CTRL_MSG_ES_ALL_PAUSE                   = 20080104,
    
        CTRL_MSG_ES_PROC_ALL_REGIST             = 20080201,
        CTRL_MSG_ES_PROC_ALL_UNREGIST           = 20080202,
        CTRL_MSG_ES_PROC_FORK_REGIST            = 20080203,
        CTRL_MSG_ES_PROC_FORK_UNREGIST          = 20080204,
        CTRL_MSG_ES_PROC_EXEC_REGIST            = 20080205,
        CTRL_MSG_ES_PROC_EXEC_UNREGIST          = 20080206,
    
        CTRL_MSG_ES_FILE_ALL_REGIST             = 20080301,
        CTRL_MSG_ES_FILE_ALL_UNREGIST           = 20080302,
        CTRL_MSG_ES_FILE_OPEN_REGIST            = 20080303,
        CTRL_MSG_ES_FILE_OPEN_UNREGIST          = 20080304,
        CTRL_MSG_ES_FILE_CREATE_REGIST          = 20080305,
        CTRL_MSG_ES_FILE_CREATE_UNREGIST        = 20080306,
        CTRL_MSG_ES_FILE_RENAME_REGIST          = 20080307,
        CTRL_MSG_ES_FILE_RENAME_UNREGIST        = 20080308,
    
        CTRL_MSG_MESSAGE_CENTER_SPEED_CHANGE    = 20090101,
        CTRL_MSG_MESSAGE_CENTER_SPEEDUP         = 20090102,
        CTRL_MSG_MESSAGE_CENTER_SLOWDOWN        = 20090103,

        CTRL_MSG_DRIVER_EVENT_REGISTER          = 20100000,
        CTRL_MSG_DRIVER_EVENT_UNREGISTER,
        CTRL_MSG_DRIVER_EVENT_UNREGISTER_ALL,
        CTRL_MSG_DRIVER_PAUSE,
        CTRL_MSG_DRIVER_RESUME,
    
        CTRL_MSG_PLUGIN_PROC_ES_OK              = 20110000,
        CTRL_MSG_PLUGIN_PROC_ES_ERR,
        CTRL_MSG_PLUGIN_PROC_PAUSE,
        CTRL_MSG_PLUGIN_PROC_RESUME,
    
        CTRL_MSG_PLUGIN_FILE_PAUSE              = 20120000,
        CTRL_MSG_PLUGIN_FILE_RESUME,
    
        CTRL_MSG_PLUGIN_NET_PAUSE               = 20130000,
        CTRL_MSG_PLUGIN_NET_RESUME,
    
        CTRL_MSG_PLUGIN_CACHE_PAUSE             = 20140000,
        CTRL_MSG_PLUGIN_CACHE_RESUME,
        CTRL_MSG_PLUGIN_CACHE_EVENT_RELOAD,
        CTRL_MSG_PLUGIN_CACHE_EVENT_UNLOAD,
    CTRL_MSG_MAXIMUM                = 29999999,
    
    MONITOR_MSG_UNKNOWN             = 30000000,
        MONITOR_MSG_COLLECTOR_CPU,
        MONITOR_MSG_COLLECTOR_MEM,
        MONITOR_MSG_COLLECTOR_FD,
        MONITOR_MSG_COLLECTOR_ES_CLIENT,
        MONITOR_MSG_COLLECTOR_PROXY_CONNECTION,
        MONITOR_MSG_COLLECTOR_PROXY_RECONNECTION,
        MONITOR_MSG_COLLECTOR_RELOAD_EVENT,
        MONITOR_MSG_COLLECTOR_RELOAD_RULE,
        MONITOR_MSG_COLLECTOR_TRACER,
        MONITOR_MSG_COLLECTOR_THREADPOOL_FULL,

        MONITOR_MSG_NET_EXTENSION_CONNECTIONS,
        MONITOR_MSG_NET_EXTENSION_MULTIPLE,
        MONITOR_MSG_NET_EXTENSION_PID,
        MONITOR_MSG_NET_EXTENSION_STATE,
    MONITOR_MSG_MAXIMUM             = 39999999,
    
    INFO_MSG_UNKNOWN                = 40000000,
        INFO_MSG_NET_EXTENSION_ENABLE,
        INFO_MSG_EVENT_ENGINES_RECONSTRUCT,
    INFO_MSG_MAXIMUM                = 49999999,
} MessageType;

typedef struct EventRegisterArgs
{
    EventType       event;
    EventObserver * observer;
} EventRegisterArgs;


typedef struct MonitorEvent
{
    time_t   time;
    uint64_t data;
    uint64_t interval;
} MonitorEvent;

inline const MonitorEvent * MSG2EVENT(const uint8_t *i_msg, size_t i_len)
{
    return (nullptr != i_msg && i_len == sizeof(MonitorEvent)) ? (MonitorEvent *)i_msg : nullptr;
}
inline MonitorEvent *MSG2EVENT(const message::MESSAGE_PTR &i_msg)
{
    return (nullptr != i_msg->buf && i_msg->len == sizeof(MonitorEvent)) ? (MonitorEvent *)i_msg->buf : nullptr;
}


#endif /* plugin_event_hpp */

