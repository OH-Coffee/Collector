
#include "event_observer_manager.hpp"

ret_code EventObserverManager::observer_add(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_ARG_ERR; break; }
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_observers);
            for(const auto it : _observers) {
                if(it == i_observer) { goto __SCOPE_END; } // 避免观察者重复
            }
            _observers.push_back(i_observer);

            _observers.sort([](EventObserver *a, EventObserver *b) {
                return (a->get_nice() < b->get_nice());
            });
        }
        __SCOPE_END: r_code = RC_OK;
    } while (false);

    return r_code;
}

ret_code EventObserverManager::observer_del(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_ARG_ERR; break; }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_observers);
        for(auto it = _observers.begin(); it != _observers.end(); it++) {
            if(*it == i_observer) {
                _observers.erase(it); r_code = RC_OK; break;
            }
        }
    } while (false);

    return r_code;
}

ret_code EventObserverManager::observer_notify(const EventPtr &i_msg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_msg) { r_code = RC_ARG_ERR; break; }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_observers);
        for (auto observer : _observers) {
            observer->event_handler(i_msg);
        }
    } while (false);

    return r_code;
}

ret_code EventObserverManager::observer_clear()
{
    std::lock_guard<std::mutex> mutex_guard(_mutex_for_observers);
    return _observers.clear(), RC_OK;
}

uint64_t EventObserverManager::observer_num()
{
    std::lock_guard<std::mutex> mutex_guard(_mutex_for_observers);
    return _observers.size();
}
