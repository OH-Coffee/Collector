
#ifndef tracepoint_engine_hpp
#define tracepoint_engine_hpp

#include "engines/bpf/bpf_engine.hpp"

class TracepointEngine : public bpf::Engine
{
public:
    bpf::LoaderArgPtr get_loader_args(EventType) override;
public:
    std::string name() override { return ENGINE_NAME_TRACEPOINT; }
};

typedef std::shared_ptr<TracepointEngine> TracepointEnginePtr;
#define NEW_TRACEPOINT_ENGINE(i_args) std::make_shared<TracepointEngine>(i_args)

#endif // tracepoint_engine_hpp
