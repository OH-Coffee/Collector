
#ifndef fanotify_engine_hpp
#define fanotify_engine_hpp

#include <memory>

#include "header/errcode.hpp"

#include "io-multiplexing/io_driver__epoll.hpp"

#include "engines/event_engine.hpp"
#include "engines/event_observer.hpp"
#include "engines/fanotify/fanotify_event_handler.hpp"

class FanotifyEngine : public EventEngine, public IO_Handler
{
public:
     FanotifyEngine();
    ~FanotifyEngine() override;
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
    std::string name() override { return ENGINE_NAME_FANOTIFY; }
public:
    int io_events()                        override;
    int io_handler(int i_fd, int i_events) override;
private:
    [[nodiscard]] ret_code fanotify_add(FanotifyEventType) const;
    [[nodiscard]] ret_code fanotify_del(FanotifyEventType) const;
private:
    std::atomic_int  _fd_fanotify{-1};
    std::atomic_bool _pause{false};
private:
    std::shared_ptr<IO_Driver> _io_driver;
private:
    std::map<FanotifyEventType, FanotifyEventHandlerPtr>    _event_handlers;
    std::mutex                                              _mutex_for_event_handlers;
};

typedef std::shared_ptr<FanotifyEngine> FanotifyEnginePtr;
#define NEW_FANOTIFY_ENGINE(i_args) std::make_shared<FanotifyEngine>(i_args)

#endif // fanotify_engine_hpp
