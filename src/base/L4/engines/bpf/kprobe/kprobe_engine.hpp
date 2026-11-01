
#ifndef kprobe_engine_hpp
#define kprobe_engine_hpp

#include "engines/bpf/bpf_engine.hpp"

class KprobeEngine : public bpf::Engine
{
public:
    bpf::LoaderArgPtr get_loader_args(EventType) override;
public:
    std::string name() override { return ENGINE_NAME_KPROBE; }
};

typedef std::shared_ptr<KprobeEngine> KprobeEnginePtr;
#define NEW_KPROBE_ENGINE(i_args) std::make_shared<KprobeEngine>(i_args)

#endif // kprobe_engine_hpp
