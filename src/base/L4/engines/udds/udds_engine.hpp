
#ifndef udds_engine_hpp
#define udds_engine_hpp

#include <memory>
#include <mutex>
#include <atomic>

#include "engines/event_engine.hpp"
#include "engines/event_observer_manager.hpp"

class UDDSEngine : public EventEngine
{
public:
    UDDSEngine();
    ~UDDSEngine() override;
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    void  pause() override { if(!_pause.load(std::memory_order_relaxed)) { _pause.store(true,  std::memory_order_relaxed); } }
    void resume() override { if( _pause.load(std::memory_order_relaxed)) { _pause.store(false, std::memory_order_relaxed); } }
public:
    ret_code   subscribe(EventType, EventObserver *) override;
    ret_code unsubscribe(EventType, EventObserver *) override;
public:
    ret_code remove_the_event(EventType)             override;
    ret_code remove_the_observer(EventObserver *)    override;
public:
    std::string name() override { return ENGINE_NAME_UDDS; }
protected:
    ret_code socket_create();
protected:
    void     socket_read_worker_thread();
private:
    std::atomic_int  _fd_socket{-1};
    std::atomic_bool _pause{false};
    std::atomic_bool _is_exit{true};
private:
    EventObserverManager _observer_mgr;
};

typedef std::shared_ptr<UDDSEngine> UDDSEnginePtr;
#define NEW_UDDS_ENGINE(i_args) std::make_shared<UDDSEngine>(i_args)

#endif // udds_engine_hpp
