
#include "message_client.hpp"
#include "message_server.hpp"

namespace message {

MessageServer::MessageServer()
{
    _is_stop = false;
}

MessageServer::~MessageServer()
{
    
}

ret_code MessageServer::init()
{
    _thread_pool_nice_high.init();
    _thread_pool_nice_high.set_queue_size(500);
    _thread_pool_nice_high.set_thread_size(1);
    _thread_pool_nice_high.set_task_interval(100);
    
    _thread_pool_nice_normal.init();
    _thread_pool_nice_normal.set_queue_size(1000);
    _thread_pool_nice_normal.set_thread_size(4);
    _thread_pool_nice_normal.set_task_interval(100);
    
    return RC_OK;
}

ret_code MessageServer::start()
{
    _thread_pool_nice_high.start();
    _thread_pool_nice_normal.start();
    
    return RC_OK;
}

ret_code MessageServer::stop()
{
    _is_stop = true;
    
    _thread_pool_nice_high.stop();
    _thread_pool_nice_normal.stop();
    
    return RC_OK;
}

ret_code MessageServer::uninit()
{
    _thread_pool_nice_high.uninit();
    _thread_pool_nice_normal.uninit();
    
    return RC_OK;
}

ret_code MessageServer::send_syn(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(_is_stop) { r_code = RC_MESSAGE_SERVER_STOPPED; break; }
        
#if defined(OS_MACOS)
        @autoreleasepool {
#endif
            for(auto &it : _clients) {
                if(RC_OK != it.second->message_ignore(i_type)) {
                    r_code = it.second->message_handler(i_type, i_buf, i_len);
                    if(r_code != RC_OK && r_code != RC_UNCONCERNED) { break; }
                }
            }
#if defined(OS_MACOS)
        }
#endif
    } while (false);
    
    return r_code;
}

ret_code MessageServer::send_syn(MESSAGE_PTR i_msg)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(_is_stop) { r_code = RC_MESSAGE_SERVER_STOPPED; break; }
        
        for(auto &it : _clients) {
            if(RC_OK != it.second->message_ignore(i_msg->type)) {
#if defined(OS_MACOS)
                @autoreleasepool {
#endif
                    r_code = it.second->message_handler(i_msg->type, i_msg->buf, i_msg->len);
                    if(r_code != RC_OK && r_code != RC_UNCONCERNED) { break; }
#if defined(OS_MACOS)
                }
#endif
            }
        }
    } while (false);
    
    return r_code;
}

ret_code MessageServer::send_asyn(MESSAGE_PTR i_msg, MessageNice i_nice)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(_is_stop) { r_code = RC_MESSAGE_SERVER_STOPPED; break; }
        
        thread::ThreadPool *thread_pool = nullptr;
        
        switch (i_nice) {
            case NICE_HIGH:   { thread_pool = &_thread_pool_nice_high;   } break;
            case NICE_NORMAL: { thread_pool = &_thread_pool_nice_normal; } break;
            default:          { thread_pool = &_thread_pool_nice_normal; } break;
        }
        
        r_code = thread_pool->add_task([this, i_msg]() {
#if defined(OS_MACOS)
            @autoreleasepool {
#endif
                for(const auto &observer : _observers) { observer->on_send_begin(i_msg); }
                
                for(auto &it : _clients) {
                    if(RC_OK != it.second->message_ignore(i_msg->type)) {
                        it.second->message_handler(i_msg->type, i_msg->buf, i_msg->len);
                    }
                }
                
                for(const auto &observer : _observers) { observer->on_send_finish(i_msg); }
#if defined(OS_MACOS)
            }
#endif
        });
    } while (false);
    
    return r_code;
}

ret_code MessageServer::add(MessageClient *i_client)
{
    if(_is_stop) { return RC_MESSAGE_SERVER_STOPPED; }
    {
        std::lock_guard<std::mutex> mutex_guard(_clients_mutex);
        _clients[i_client->name()] = i_client;
    }
    return RC_OK;
}

ret_code MessageServer::remove(MessageClient *i_client)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(_is_stop) { r_code = RC_MESSAGE_SERVER_STOPPED; break; }
        
        std::lock_guard<std::mutex> mutex_guard(_clients_mutex);
        
        if(_clients.find(i_client->name()) != _clients.end()) {
            _clients.erase(i_client->name()); r_code = RC_OK;
        }
    } while (false);
    
    return r_code;
}

ret_code MessageServer::add_observer(MessageObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;
    
    if(nullptr == i_observer) {
        r_code = RC_ARG_ERR;
    } else {
        r_code = RC_OK; _observers.push_back(i_observer);
    }
    
    return r_code;
}

}   // namespace message

