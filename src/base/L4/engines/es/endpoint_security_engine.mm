
#include "engines/es/endpoint_security_engine.hpp"

ret_code EndpointSecurityEngine::init()
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(nil == _es_client) { _es_client = [[ES_Client alloc] init];  }
        if(nil == _es_client) { r_code = RC_ES_NOT_INIT; break;         }
        
        [_es_client clientInit:^(es_client_t *i_client, const es_message_t *i_msg) { @autoreleasepool {
            if(_pause.load(std::memory_order_relaxed)) { return; }
            
            static const int64_t body_len = sizeof(EventBody);
            
            auto msg  = NEW_EVENT_MESSAGE();
            auto head = &msg->head;
            auto body = &msg->body;
            
            head->event_type        = static_cast<EventType>(i_msg->event_type);
            head->event_producer    = EP_ENDPOINT_SECURITY;
            head->event_process.pid = audit_token_to_pid(i_msg->process->audit_token);
            head->event_process.uid = audit_token_to_euid(i_msg->process->audit_token);
            head->event_body_len    = body_len;
            
            body->es = NEW_EVENT_BODY_ENDPOINT_SECURITY(i_client, i_msg);
            
            _source[i_msg->event_type].observer_notify(msg);
        } }];
        
        // 在macOS中EventType := es_event_type_t
        es_event_type_t events[] =
        {
            (es_event_type_t)EVENT_TYPE_FORK,
            (es_event_type_t)EVENT_TYPE_EXIT_GROUP,
            
            (es_event_type_t)EVENT_TYPE_EXEC,
//            (es_event_type_t)EVENT_TYPE_OPEN,
            (es_event_type_t)EVENT_TYPE_CLOSE,
            (es_event_type_t)EVENT_TYPE_CREATE,
            (es_event_type_t)EVENT_TYPE_RENAME,
        };
        r_code = [_es_client eventRegist:events withNumber:( sizeof(events) / sizeof(events[0]))];
    } while (false);
    
    return r_code;
}

ret_code EndpointSecurityEngine::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = RC_OK;
    } while (false);
    
    return r_code;
}

ret_code EndpointSecurityEngine::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nil != _es_client) {
            r_code = [_es_client eventAllUnregist];
        }
    } while(false);

    return r_code;
}

ret_code EndpointSecurityEngine::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->stop();
        
        if(nil != _es_client) {
            r_code = [_es_client clientUninit];
        }
    } while(false);

    return r_code;
}

ret_code EndpointSecurityEngine::subscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = _source[i_event].observer_add(i_observer);
    } while (false);

    return r_code;
}

ret_code EndpointSecurityEngine::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = _source[i_event].observer_del(i_observer);
    } while (false);

    return r_code;
}

ret_code EndpointSecurityEngine::remove_the_event(EventType i_event)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = _source[i_event].observer_clear();
    } while (false);

    return r_code;
}

ret_code EndpointSecurityEngine::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    for(auto &observer_manager : _source) {
        r_code = (RC_OK == observer_manager.observer_del(i_observer)) ? RC_OK : r_code;
    }
    
    return r_code;
}
