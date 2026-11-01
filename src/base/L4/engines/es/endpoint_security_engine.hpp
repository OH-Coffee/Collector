
#ifndef es_engine_h
#define es_engine_h

#include <map>
#include <memory>

#include "header/errcode.hpp"
#include "es/es_client.hpp"
#include "engines/event_engine.hpp"
#include "engines/event_observer_manager.hpp"

class EndpointSecurityEngine : public EventEngine
{
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    void resume() override { if( _pause.load(std::memory_order_relaxed)) { _pause.store(false, std::memory_order_relaxed); } }
    void  pause() override { if(!_pause.load(std::memory_order_relaxed)) { _pause.store(true,  std::memory_order_relaxed); } }
public:
    ret_code   subscribe(EventType, EventObserver *) override;
    ret_code unsubscribe(EventType, EventObserver *) override;
public:
    ret_code remove_the_event(EventType)             override;
    ret_code remove_the_observer(EventObserver *)    override;
public:
    std::string name() override { return ENGINE_NAME_ES; }
private:
    ES_Client *_es_client{nil};
private:
    std::array<EventObserverManager, ES_EVENT_TYPE_LAST> _source;
private:
    std::atomic_bool _pause{false};
};

typedef std::shared_ptr<EndpointSecurityEngine> EndpointSecurityEnginePtr;
#define NEW_ENDPOINT_SECURITY_ENGINE(i_args...) std::make_shared<EndpointSecurityEngine>(i_args)

#endif /* es_engine_h */
