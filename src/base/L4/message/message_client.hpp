
#ifndef message_client_hpp
#define message_client_hpp

#include "message/message.hpp"
#include "message/message_public.hpp"

namespace message {

class MessageServer;

class MessageClient
{
public:
    MessageClient(const std::string &i_name, MessageServer *i_server);
    virtual ~MessageClient() = default;
public:
    ret_code send(uint64_t i_type, uint8_t *i_buf, size_t i_len, MessageNice i_nice = NICE_NORMAL);
    ret_code send_syn(uint64_t i_type, uint8_t *i_buf, size_t i_len);
public:
    ret_code send_syn(MESSAGE_PTR i_msg);
    ret_code send_asyn(MESSAGE_PTR i_msg, MessageNice i_nice = NICE_NORMAL);
public:
    ret_code enable();
    ret_code disable();
public:
    ret_code is_full();
public:
    ret_code message_clear();
    ret_code message_regist(uint64_t i_type);
    ret_code message_ignore(uint64_t i_type);
public:
    virtual ret_code message_handler(uint64_t i_type, uint8_t *i_buf, size_t i_len) = 0;
public:
    const std::string &name()       { return _name; }
private:
    std::string         _name;
    MessageServer*      _server;
    std::set<uint64_t> _message_types;
};

}   // namespace message

#endif /* message_client_hpp */

