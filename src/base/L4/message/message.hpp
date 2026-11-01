
#ifndef message_hpp
#define message_hpp

#include "header/pch.hpp"

namespace message {

class Message
{
public:
    Message(uint64_t i_type, uint8_t *i_buf, size_t i_len)
    {
        type = i_type;
        
        if(i_len > 0) {
            len  = i_len;
            buf  = new uint8_t[i_len]; bzero(buf, i_len);
            
            if(nullptr != i_buf) { memcpy(buf, i_buf, i_len); }
        } else {
            len = 0;
            buf = nullptr;
        }
    }
    ~Message()
    {
        len = 0;
        
        if(nullptr != buf) { delete[] buf; buf = nullptr; }
    }
public:
    uint64_t type;
    size_t   len;
    uint8_t  *buf;
};

typedef std::shared_ptr<Message>            MESSAGE_PTR;
#define NEW_MESSAGE(i_type, i_buf, i_len)   std::make_shared<message::Message>(i_type, (uint8_t *)i_buf, i_len)

} // namespace message


#endif /* message_hpp */
