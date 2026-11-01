
#ifndef bpf_loader_h
#define bpf_loader_h

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include <unistd.h>

#include "libbpf.h"

#include "engines/bpf/bpf_link_type.hpp"
#include "engines/bpf/bpf_prog_info.hpp"

#include "header/errcode.hpp"
#include "io-multiplexing/io_driver.hpp"
#include "io-multiplexing/io_handler.hpp"

#include "engines/event_observer.hpp"
#include "engines/event_observer_manager.hpp"
#include "engines/bpf/bpf_linker.hpp"

namespace bpf { // namespace bpf

typedef struct LoaderArg
{
public:
    ProgInfo    prog_info;
    LinkArgs    link_args;
public:
    void clear() {
        prog_info.clear();
        link_args.clear();
    }
} LoaderArg;
typedef std::shared_ptr<LoaderArg>  LoaderArgPtr;
#define NEW_BPF_LOADER_ARG(i_args)  std::make_shared<bpf::LoaderArg>(i_args)


class Loader : public IO_Handler
{
public:
    friend void bpf_coll_callback(void *, int, void *, __u32);
    friend void bpf_lost_callback(void *, int, __u64);
public:
    explicit Loader(IO_Driver *);
    virtual ~Loader();
public:
    ret_code init(const ProgInfo &);
    ret_code start();
    ret_code stop();
    ret_code uninit();
public:
    void  pause() { if(!_pause.load(std::memory_order_relaxed)) { _pause.store(true,  std::memory_order_relaxed); } }
    void resume() { if( _pause.load(std::memory_order_relaxed)) { _pause.store(false, std::memory_order_relaxed); } }
public:
    ret_code   subscribe(EventType, EventObserver *, const LinkArgs &);
    ret_code unsubscribe(EventType, EventObserver *);
public:
    ret_code remove_the_event(EventType);
    ret_code remove_the_observer(EventObserver *);
public:
    [[nodiscard]] int fd() const { return _bpf_buff_fd; }
public:
    int io_events()          override;
    int io_handler(int, int) override;
private:
    void event_message_coll_handler(const Event *i_msg);
    void event_message_lost_handler(uint64_t i_lost_counter);
private:
    std::atomic_bool _pause{false};
private:
    IO_Driver *_io_driver;
private:
    ProgInfo                                    _bpf_arg;
    struct bpf_object                          *_bpf_obj;
    struct perf_buffer                         *_bpf_buff;
    int                                         _bpf_buff_fd;
    std::map<EventType, LinkGroupPtr>           _bpf_linkers;
    std::mutex                                  _mutex_for_bpf_attr;
};
typedef std::shared_ptr<Loader> LoaderPtr;
#define NEW_BPF_LOADER(i_args)  std::make_shared<bpf::Loader>(i_args)

} // namespace bpf

#endif /* bpf_loader_h */
