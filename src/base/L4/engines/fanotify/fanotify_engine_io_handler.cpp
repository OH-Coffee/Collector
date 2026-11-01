
#include <strings.h>
#include <sys/fanotify.h>

#include "engines/fanotify/fanotify_engine.hpp"

int FanotifyEngine::io_events()
{
    return IO_EVENT_READ;   // 用于告知IO-Driver，仅关注可读事件
}

int FanotifyEngine::io_handler(int i_fd, int i_events)
{
    int r_code = 0;

    do {
        struct fanotify_event_metadata fan_data[32];
        bzero(fan_data, sizeof(fan_data));

        ssize_t fan_data_len = read(i_fd, fan_data, sizeof(fan_data));
        if(fan_data_len <= 0) { break; }

        struct fanotify_event_metadata *fan_data_ptr = fan_data;
        while (FAN_EVENT_OK(fan_data_ptr, fan_data_len)) {
            if(fan_data_ptr->vers == FANOTIFY_METADATA_VERSION && !_pause.load(std::memory_order_relaxed)) {
                std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

                for(const auto& it : _event_handlers) {
                    if(it.first == (fan_data_ptr->mask & it.first)) {
                        it.second->event_handler(fan_data_ptr);
                    }
                }
            }
            close(fan_data_ptr->fd);
            fan_data_ptr = FAN_EVENT_NEXT(fan_data_ptr, fan_data_len);
        }
    } while (false);

    return r_code;
}
