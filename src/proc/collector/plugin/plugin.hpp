
#ifndef plugin_hpp
#define plugin_hpp

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "header/consts.hpp"
#include "header/errcode.hpp"

#include "sleep/sleep.hpp"
#include "logger/logger.hpp"
#include "filter/filter.hpp"
#include "message/message_client.hpp"

namespace global
{
    filter::Filter& filter();
}

typedef enum : uint64_t
{
    STATE_UNKNOWN   = 0,
    STATE_NO_LOAD   = 1,
    STATE_NO_PERM   = 2,
    STATE_READY     = 3,
    STATE_RUNNING   = 4,
    STATE_STOPPED   = 5,
    STATE_FAILURE   = 6,
} PluginState;

class Plugin : public message::MessageClient
{
public:
     Plugin(const std::string &i_name, message::MessageServer *i_server) : message::MessageClient(i_name, i_server) {}
    ~Plugin() override = default;
public:
    virtual ret_code init()     = 0;
    virtual ret_code start()    = 0;
    virtual ret_code stop()     = 0;
    virtual ret_code uninit()   = 0;
public:
    virtual PluginState state()   { return STATE_UNKNOWN; }
    virtual std::string version() { return "";            }
public:
    virtual ret_code wakeup()                   { return _sleep.wakeup();         }
    virtual ret_code sleep_ms(uint64_t i_time)  { return _sleep.sleep_ms(i_time); }
public:
    void     exit()             { this->exit(true);                         }
    void     exit(bool i_bool)  { _is_exit = i_bool; this->wakeup();        }
    ret_code is_exit()          { return ( _is_exit ? RC_OK : RC_FAIL);     }
public:
    ret_code msg_send(const message::MESSAGE_PTR & i_msg);
    ret_code msg_send(uint64_t i_type, uint8_t *i_buf, size_t i_len);
private:
    std::atomic_bool _is_exit = ATOMIC_VAR_INIT(false);
private:
    Sleep _sleep;
};

class PluginManager : public Plugin
{
public:
     PluginManager(const std::string &i_name, message::MessageServer *i_server) : Plugin(i_name, i_server) {}
    ~PluginManager() override = default;
};


#endif /* plugin_hpp */

