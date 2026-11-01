
#include "config/collector_config.hpp"

#include "plugin/plugin_helper.hpp"
#include "plugins-data/net/plugin_net.hpp"

static CollectorConfig *g_conf = &CollectorConfig::ins();

#if defined(OS_MACOS)

ret_code PluginNet::event_handler(SCHEMA &io_msg)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(SWITCH_ON != _plugin_switch.load() || _event_pause) { r_code = RC_UNCONCERNED; break; }
        
        if (unlikely(MESSAGE_EXCEPTION == io_msg.Header.MessageType)) {
            r_code = RC_OK;
            break;
        }
        switch (io_msg.Header.OperType) {
            case OP_NET_UDP_DNS     : { r_code = event_handler_for_dns(io_msg); } break;
            case OP_NET_CONNECTION  : { r_code = event_handler_for_net(io_msg); } break;
            default: break;
        }
    } while (false);
    
    return r_code;
}

#elif defined(OS_LINUX)

#include "plugins-data/net/plugin_net_event_handler_linux/handlers.hpp"

using namespace cardinal::plugins::plugin_net::handlers;

namespace {
    typedef ret_code (*event_handler)(const EventPtr &i_msg, SCHEMA *o_msg);
}

static std::map<EventType, event_handler> g_event_handler_map = {
    { EVENT_TYPE_CONNECT,           event_handler_for_connect   },
    { EVENT_TYPE_RECVMSG,           event_handler_for_recvmsg   },
    { EVENT_TYPE_RECVFROM,          event_handler_for_recvfrom  },
    { EVENT_TYPE_SKB_CONSUME_UDP,   event_handler_for_skb_consume_udp  },
#ifdef ENABLE_NSS_EXTENSION
    { EVENT_TYPE_UDDS,              event_handler_for_udds_dns  },
#endif
};

ret_code PluginNet::event_handler(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_event_pause) { r_code = RC_UNCONCERNED; break; }
        
        if(_threads.get_thread_size() <= 0) {
            r_code = this->event_handler_run(i_msg);
        } else {
            r_code = _threads.add_task([this, i_msg](){
                this->event_handler_run(i_msg);
            });
        }
    } while (false);

    return r_code;
}

ret_code PluginNet::event_handler_run(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        static std::atomic_uint64_t lost_counter         = 0;
        static       const uint64_t lost_counter_modular = g_conf->get_uint64("plugins.lost_counter_modular");

        if(RC_OK == MessageClient::is_full())  {
            ++lost_counter; r_code = RC_THREAD_POLL_TASK_QUEUE_FULL; break;
        }

        auto event = g_event_handler_map.find(i_msg->head.event_type);
        if(event == g_event_handler_map.end()) {
            ++lost_counter; r_code = RC_UNCONCERNED; break;
        }

        SCHEMA msg; schema_init(&msg, sizeof(msg));

        r_code = event->second(i_msg, &msg);

        if(RC_OK == r_code) {
            r_code = MessageClient::send(DATA_MSG_SEND_TO_PROXY, (uint8_t *)&msg, sizeof(msg));
            if(RC_OK != r_code && (++lost_counter % lost_counter_modular == 0)) {
                LOG_WARN("[PluginNet]\t[ NO] 数据消息发送 -> 累计丢失数：%lld; 错误码：%d", lost_counter.load(), r_code);
            }
        } else if( !(RC_FILTER_MATCHED == r_code || RC_UNCONCERNED == r_code) ) {
            if(0 == (++lost_counter % lost_counter_modular)) {
                LOG_WARN("[PluginNet]\t[ NO] 事件采集完成 -> 累计丢失数：%lld; 错误码：%d", lost_counter.load(), r_code);
            }
        }
    } while (false);

    return r_code;
}

#endif
