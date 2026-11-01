
#ifndef message_server_hpp
#define message_server_hpp

#include "thread/thread_pool.hpp"

#include "message/message.hpp"
#include "message/message_public.hpp"
#include "message/message_observer.hpp"

namespace message {

class MessageClient;

class MessageServer final
{
    friend class MessageClient;
public:
    MessageServer();
    ~MessageServer();
public:
    ret_code init();
    ret_code start();
    ret_code stop();
    ret_code uninit();
public:
    ret_code    add(MessageClient *i_client);
    ret_code remove(MessageClient *i_client);
public:
    ret_code add_observer(MessageObserver *i_observer);
private:
    ret_code send_syn(uint64_t i_type, uint8_t *i_buf, size_t i_len);
private:
    ret_code send_syn(MESSAGE_PTR i_msg);
    ret_code send_asyn(MESSAGE_PTR i_msg, MessageNice i_nice = NICE_NORMAL);
private:
    ret_code is_full() { return _thread_pool_nice_normal.is_full(); }

private:
    std::atomic_bool _is_stop;
private:
    std::map<std::string, MessageClient*>   _clients;
    std::mutex                              _clients_mutex;
private:
    thread::ThreadPool _thread_pool_nice_high;
    thread::ThreadPool _thread_pool_nice_normal;
private:
    std::list<MessageObserver *> _observers;
};

}   // namespace: message



#endif /* message_server_hpp */

