
#ifndef event_engine_factory_hpp
#define event_engine_factory_hpp

#include "engines/event_engine.hpp"

class EventEngineFactory
{
public:
    static EventEnginePtr create(const std::string &);
};

#endif // event_engine_factory_hpp
