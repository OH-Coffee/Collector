
#include "message/message_client.hpp"
#include "message/message_server.hpp"

namespace message {

MessageClient::MessageClient(const std::string &i_name, MessageServer *i_server)
{
    _name   = i_name;
    _server = i_server;
}

ret_code MessageClient::send(uint64_t i_type, uint8_t *i_buf, size_t i_len, MessageNice i_nice)
{
    return _server->send_asyn(NEW_MESSAGE(i_type, i_buf, i_len), i_nice);
}

ret_code MessageClient::send_syn(uint64_t i_type, uint8_t *i_buf, size_t i_len)
{
    return _server->send_syn(i_type, i_buf, i_len);
}

ret_code MessageClient::send_syn(MESSAGE_PTR i_msg)
{
    return _server->send_syn(i_msg);
}

ret_code MessageClient::send_asyn(MESSAGE_PTR i_msg, MessageNice i_nice)
{
    return _server->send_asyn(i_msg, i_nice);
}


ret_code MessageClient::is_full()
{
    return _server->is_full();
}

ret_code MessageClient::message_clear()
{
    _message_types.clear(); return RC_OK;
}

ret_code MessageClient::message_regist(uint64_t i_type)
{
    _message_types.insert(i_type); return RC_OK;
}

ret_code MessageClient::message_ignore(uint64_t i_type)
{
    return ( _message_types.find(i_type) == _message_types.end() ? RC_OK : RC_FAIL);
}

}   // namespace message

