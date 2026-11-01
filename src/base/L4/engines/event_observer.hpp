
#ifndef event_observer_hpp
#define event_observer_hpp

// 不要删除以下任意一个头文件，也不要调整头文件顺序

#include <memory>
#include <string>

#include <sys/socket.h>

#include "header/errcode.hpp"
#include "event/event.h"

#define EVENT_OBSERVER_NAME_DEFAULT "cardinal.event-observer.unknown"


typedef std::shared_ptr<Event>  EventPtr;
#define NEW_EVENT_MESSAGE(i_args...) std::make_shared<Event>(i_args)


typedef enum : int64_t
{
    EVENT_OBSERVER_NICE_UNKNOWN     = 0,
    EVENT_OBSERVER_NICE_HIGH        = 10000,
    EVENT_OBSERVER_NICE_MIDDLE      = 20000,
    EVENT_OBSERVER_NICE_LOW         = 30000,
} EventObserverNice;


class EventObserver
{
public:
             EventObserver() = default;
    virtual ~EventObserver() = default;
public:
    virtual ret_code event_handler(const EventPtr &) = 0;
public:
    void                        set_nice(uint64_t i_nice)           { _nice = i_nice; }
    [[nodiscard]] uint64_t      get_nice() const                    { return _nice;   }
public:
    void                        set_name(const std::string &i_name) { _name = i_name; }
    [[nodiscard]] std::string   get_name() const                    { return _name;   }
private:
    uint64_t    _nice = EVENT_OBSERVER_NICE_MIDDLE;
    std::string _name = EVENT_OBSERVER_NAME_DEFAULT;
};
typedef std::shared_ptr<EventObserver> EventObserverPtr;


#endif // event_observer_hpp
