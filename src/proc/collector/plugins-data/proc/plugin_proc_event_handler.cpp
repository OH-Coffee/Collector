
#include "plugins-data/proc/plugin_proc_event_handler.hpp"

namespace {
    typedef ret_code (*event_handler)(const EventPtr &i_msg, SCHEMA *o_msg);
}

static std::map<EventType, event_handler> g_event_handler_map =
{
    { EVENT_TYPE_EXEC,    event_handler_for_exec },
};


ret_code PluginProc::event_handler(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_is_pause) { break; }
        
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

ret_code PluginProc::event_handler_run(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        static std::atomic_uint64_t lost_counter = 0;

        if(RC_OK == MessageClient::is_full()) {
            ++lost_counter; break;
        }

        auto event = g_event_handler_map.find(i_msg->head.event_type);
        if(event == g_event_handler_map.end()) {
            ++lost_counter; break;
        }

        SCHEMA msg; schema_init(&msg, sizeof(msg));

        r_code = event->second(i_msg, &msg);

        if(RC_OK == r_code) {
            r_code = MessageClient::send(DATA_MSG_SEND_TO_PROXY, (uint8_t *)&msg, sizeof(msg));
            if(RC_OK != r_code && ++lost_counter % 100 == 0) {
                LOG_ERROR("[PluginProc]\t[ NO] 数据消息发送 -> 累计丢失数：%lld; 错误码：%d", lost_counter.load(), r_code);
            }
        } else if( !(RC_FILTER_MATCHED == r_code || RC_UNCONCERNED == r_code) ) {
            if (0 == (++lost_counter % 100)) {
                LOG_WARN("[PluginProc]\t[ NO] 事件采集完成 -> PID：%d，累计丢失数：%lld; 错误码：%d",
                         i_msg->head.event_process.pid,
                         lost_counter.load(), r_code);
            }
        }
    } while (false);

    return r_code;
}
