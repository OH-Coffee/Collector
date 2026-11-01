
#ifndef bpf_engine_h
#define bpf_engine_h

#include <map>
#include <memory>

#include "header/errcode.hpp"
#include "io-multiplexing/io_driver__epoll.hpp"
#include "engines/event_engine.hpp"
#include "engines/bpf/bpf_loader.hpp"

namespace bpf { // namespace bpf

class Engine : public EventEngine
{
public:
    ret_code init()     override;
    ret_code start()    override;
    ret_code stop()     override;
    ret_code uninit()   override;
public:
    void  pause() override;
    void resume() override;
public:
    ret_code   subscribe(EventType, EventObserver *) override;
    ret_code unsubscribe(EventType, EventObserver *) override;
public:
    ret_code remove_the_event(EventType)             override;
    ret_code remove_the_observer(EventObserver *)    override;
protected:
    virtual LoaderArgPtr get_loader_args(EventType) = 0;
private:
    std::shared_ptr<IO_Driver> _io_driver;
private:
    std::map<std::string, LoaderPtr>    _event_sources;
    std::mutex                          _mutex_for_event_sources;
};

typedef std::shared_ptr<Engine> EnginePtr;
#define NEW_BPF_ENGINE(i_args) std::make_shared<bpf::Engine>(i_args)

int64_t change_buffer_size(int64_t i_size);

} // namespace bpf


#endif /* bpf_engine_h */
