
#include "engines/bpf/bpf_loader.hpp"

namespace bpf { // namespace bpf

void bpf_coll_callback(void *i_args, int i_cpu, void *i_data, __u32 i_data_size)
{
    auto handler = (Loader *)i_args;
    if(nullptr != handler) {
        handler->event_message_coll_handler((Event *)i_data);
    }
}
void bpf_lost_callback(void *i_args, int i_cpu, __u64 i_lost_count)
{
    auto handler = (Loader *)i_args;
    if(nullptr != handler) {
        handler->event_message_lost_handler(i_lost_count);
    }
}

Loader::Loader(IO_Driver *i_io_driver)
{
    _io_driver      = i_io_driver;
    _bpf_obj        = nullptr;
    _bpf_buff       = nullptr;
    _bpf_buff_fd    = -1;
}
Loader::~Loader()
{
    this->uninit();
}

ret_code Loader::init(const ProgInfo &i_arg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_arg.path.empty() || i_arg.buff_name.empty())  {
            r_code = RC_ARG_ERR; break;
        }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        if(nullptr == _io_driver)  { r_code = RC_IO_DRIVER_NULL;      break; }
        if(!_bpf_arg.path.empty()) { r_code = RC_BPF_LOADER_INIT_ERR; break; }

        _bpf_arg = i_arg; r_code = RC_OK;
    } while (false);

    return r_code;
}
ret_code Loader::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);
        if(_bpf_obj != nullptr && _bpf_buff_fd >= 0) {
            r_code = RC_OK; break;  // BPF程序已启动
        }

        if(nullptr == _io_driver) {
            r_code = RC_IO_DRIVER_NULL; break;
        }

        _bpf_obj = bpf_object__open(_bpf_arg.path.c_str());
        if(libbpf_get_error(_bpf_obj)) {
            r_code = RC_BPF_OBJ_OPEN_ERR; break;
        }

        if(bpf_object__load(_bpf_obj)) {
            r_code = RC_BPF_OBJ_LOAD_ERR; break;
        }

        int bpf_map_fd = bpf_object__find_map_fd_by_name(_bpf_obj, _bpf_arg.buff_name.c_str());
        if(bpf_map_fd < 0) {
            r_code = RC_BPF_OBJ_FIND_ERR; break;
        }

        _bpf_buff = perf_buffer__new(bpf_map_fd,
                                     _bpf_arg.buff_size,
                                     bpf_coll_callback,
                                     bpf_lost_callback,
                                     (void *)this,
                                     nullptr);
        if (libbpf_get_error(_bpf_buff)) {
            r_code = RC_BPF_PERF_NEW_ERR; break;
        }

        _bpf_buff_fd = perf_buffer__epoll_fd(_bpf_buff);
        if(_bpf_buff_fd < 0) {
            r_code = RC_BPF_PERF_EPOLL_FD_ERR; break;
        }

        r_code = (nullptr != _io_driver) ? _io_driver->insert(_bpf_buff_fd, this) : RC_IO_DRIVER_NULL;
    } while (false);

    return r_code;
}
ret_code Loader::stop()
{
    ret_code r_code = RC_UNKNOWN;
    do {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        if(nullptr != _io_driver)       { r_code = _io_driver->erase(_bpf_buff_fd, this);       }
        if(_bpf_buff_fd >= 0)           { close(_bpf_buff_fd);          _bpf_buff_fd = -1;      }
        if(nullptr != _bpf_buff)        { perf_buffer__free(_bpf_buff); _bpf_buff    = nullptr; }
        if(nullptr != _bpf_obj)         { bpf_object__close(_bpf_obj);  _bpf_obj     = nullptr; }

        for(const auto &it : _bpf_linkers) {
            it.second->unlink();
        } _bpf_linkers.clear();

        r_code = RC_OK; break;
    } while (false);

    return r_code;
}
ret_code Loader::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->stop();
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);
            _bpf_arg.clear();
            _io_driver = nullptr;
        }
    } while (false);

    return r_code;
}

ret_code Loader::subscribe(EventType i_event, EventObserver *i_observer, const LinkArgs &i_args)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer || i_args.empty())                     { r_code = RC_ARG_ERR; break; }
        if(EVENT_TYPE_UNKNOWN == i_event || EVENT_TYPE_ALL == i_event)  { r_code = RC_ARG_ERR; break; }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        LinkGroupPtr linker = nullptr;
        {
            auto it = _bpf_linkers.find(i_event);
            if(_bpf_linkers.end() != it) {
                linker = it->second;
            } else {
                linker = NEW_BPF_LINK_GROUP(_bpf_obj);
                if(RC_OK != (r_code = linker->link(i_args))){
                    break;
                } else {
                    _bpf_linkers[i_event] = linker;
                }
            }
        }
        r_code = (nullptr == linker) ? RC_BPF_LINKER_NULL : linker->observer_add(i_observer);
    } while (false);

    return r_code;
}
ret_code Loader::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_observer == nullptr || i_event == EVENT_TYPE_ALL || i_event == EVENT_TYPE_UNKNOWN) {
            r_code = RC_ARG_ERR; break;
        }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        LinkGroupPtr linker = nullptr;
        {
            auto it = _bpf_linkers.find(i_event);
            if(_bpf_linkers.end() != it) {
                linker = it->second;
            }
        }
        if(nullptr == linker) { r_code = RC_BPF_LINKER_NULL; break; }

        r_code = linker->observer_del(i_observer);
        if(linker->observer_num() <= 0) {
            linker->unlink(); _bpf_linkers.erase(i_event);
        }
    } while (false);

    return r_code;
}

ret_code Loader::remove_the_event(EventType i_event)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_event == EVENT_TYPE_ALL || i_event == EVENT_TYPE_UNKNOWN) {
            r_code = RC_ARG_ERR; break;
        }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        LinkGroupPtr linker = nullptr;
        {
            auto it = _bpf_linkers.find(i_event);
            if(_bpf_linkers.end() != it) {
                linker = it->second;
            }
        }
        if(nullptr == linker) { r_code = RC_BPF_LINKER_NULL; break; }

        r_code = linker->unlink();
        r_code = (_bpf_linkers.erase(i_event) > 0) ? RC_OK : r_code;
    } while (false);

    return r_code;
}
ret_code Loader::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_ARG_ERR; break; }

        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        LinkGroupPtr linker = nullptr;
        for(auto it = _bpf_linkers.begin(); it != _bpf_linkers.end(); ) {
            if(nullptr == (linker = it->second)) { ++it; continue; }

            if(RC_OK == linker->observer_del(i_observer)) {
                r_code = RC_OK;
            }

            it = (linker->observer_num() <= 0) ? _bpf_linkers.erase(it) : ++it;
        }
    } while (false);

    return r_code;
}


int Loader::io_events()
{
    return IO_EVENT_READ;
}
int Loader::io_handler(int i_fd, int i_events)
{
    return (i_fd == _bpf_buff_fd) ? perf_buffer__consume(_bpf_buff) : -1;
}

void Loader::event_message_coll_handler(const Event *i_msg)
{
    if(_pause.load(std::memory_order_relaxed)) { return; }

    LinkGroupPtr linker = nullptr;
    {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_bpf_attr);

        auto it = _bpf_linkers.find(i_msg->head.event_type);
        linker  = (_bpf_linkers.end() != it) ? it->second : nullptr;
    }

    if(nullptr != linker) {
        auto msg = NEW_EVENT_MESSAGE();
        {
            msg->head = i_msg->head;
            memcpy(&msg->body, &i_msg->body, i_msg->head.event_body_len);
        }
        linker->observer_notify(msg);
    }
}
void Loader::event_message_lost_handler(uint64_t i_lost_counter)
{
//    if(_pause.load(std::memory_order_relaxed)) { return; }
}

} // namespace bpf
