
#ifndef message_observer_hpp
#define message_observer_hpp

#include "header/pch.hpp"

#include "message/message.hpp"

namespace message {

class MessageObserver
{
public:
    virtual ret_code on_send_begin(MESSAGE_PTR i_msg_ptr)  = 0;
    virtual ret_code on_send_finish(MESSAGE_PTR i_msg_ptr) = 0;
};

}

#endif /* message_observer_hpp */
