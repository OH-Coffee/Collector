
#ifndef event_observer_manager_hpp
#define event_observer_manager_hpp

#include <list>
#include <mutex>

#include "header/errcode.hpp"
#include "engines/event_engine.hpp"
#include "engines/event_observer.hpp"

class EventObserverManager
{
public:
             EventObserverManager() = default;
    virtual ~EventObserverManager() = default;
public:
    virtual ret_code observer_add(EventObserver *);
    virtual ret_code observer_del(EventObserver *);
public:
    virtual ret_code observer_notify(const EventPtr &);
    virtual ret_code observer_clear();
    virtual uint64_t observer_num();
private:
    std::list<EventObserver *>  _observers;
    std::mutex                  _mutex_for_observers;
};

#endif // event_observer_manager_hpp
