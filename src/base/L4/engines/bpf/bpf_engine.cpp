
#include <thread>

#include "system/sys_info.hpp"
#include "engines/bpf/bpf_prog_info.hpp"
#include "engines/bpf/bpf_engine.hpp"
#include "engines/bpf/bpf_prog_info.hpp"

namespace bpf { // namespace bpf

ret_code Engine::init()
{
    ret_code r_code = RC_UNKNOWN;
    do {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);

        // 创建IO驱动器，为事件源提供驱动能力。
        _io_driver = std::make_shared<IO_DriverEpoll>();
        if(nullptr == _io_driver) {
            r_code = RC_IO_DRIVER_CREATE_ERR; break;
        }

        r_code = _io_driver->init(); // IO驱动器初始化
    } while (false);
    return r_code;
}

ret_code Engine::start()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 创建线程，IO驱动器开始工作
        std::thread thread_io_driver([this](){
            this->_io_driver->start();
        });
        thread_io_driver.detach();

        r_code = RC_OK; break;
    } while (false);

    return r_code;
}

ret_code Engine::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);
            for(const auto& it : _event_sources) { it.second->stop(); }
        }
        r_code = _io_driver->stop();
    } while(false);

    return r_code;
}

ret_code Engine::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);
            for(const auto& it : _event_sources) {
                it.second->uninit();
            } _event_sources.clear();
        }
        r_code = _io_driver->uninit();
    } while(false);

    return r_code;
}

void Engine::pause()
{
    std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);
    for(const auto& it : _event_sources) {
        it.second->pause();
    }
}
void Engine::resume()
{
    std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);
    for(const auto& it : _event_sources) {
        it.second->resume();
    }
}

ret_code Engine::subscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        auto loader_args = this->get_loader_args(i_event);
        if(nullptr == loader_args) {
            r_code = RC_UNCONCERNED; break;
        }

        LoaderPtr loader = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);

            auto it = _event_sources.find(loader_args->prog_info.path);
            if(it != _event_sources.end()) {
                loader = it->second; r_code = RC_OK;
            } else {
                // 事件源没有自驱能力，需要将其绑定到IO驱动器上
                loader = NEW_BPF_LOADER(_io_driver.get());

                if(RC_OK != (r_code = loader->init(loader_args->prog_info)))    { break; }
                if(RC_OK != (r_code = loader->start()))                         { break; }

                _event_sources.insert({ loader_args->prog_info.path, loader });
            }
        }
        if(nullptr != loader && RC_OK == r_code) {
            r_code = loader->subscribe(i_event, i_observer, loader_args->link_args);
        }
    } while (false);

    return r_code;
}

ret_code Engine::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if((nullptr == i_observer) || (i_event == EVENT_TYPE_UNKNOWN) || (i_event == EVENT_TYPE_ALL)) {
            r_code = RC_ARG_ERR; break;
        }

        // 通过此方法来判断事件码是否可被此事件引擎受理
        auto loader_args = this->get_loader_args(i_event);
        if(nullptr == loader_args) {
            r_code = RC_UNCONCERNED; break;
        }

        LoaderPtr loader = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);

            auto it = _event_sources.find(loader_args->prog_info.path);
            loader  = (_event_sources.end() != it) ? it->second : nullptr;
        }
        if(nullptr == loader) { r_code = RC_BPF_LOADER_NULL; break; }

        r_code = loader->unsubscribe(i_event, i_observer);
    } while (false);

    return r_code;
}

ret_code Engine::remove_the_event(EventType i_event)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        auto loader_args = this->get_loader_args(i_event);
        if(nullptr == loader_args) {
            r_code = RC_UNCONCERNED; break;
        }

        if((i_event != EVENT_TYPE_UNKNOWN) && (i_event != EVENT_TYPE_ALL)) {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);

            for(const auto &it : _event_sources) {
                r_code = (RC_OK == it.second->remove_the_event(i_event) ? RC_OK : r_code);
            }
        }
    } while (false);

    return r_code;
}
ret_code Engine::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    if(nullptr != i_observer) {
        std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_sources);

        for(const auto &it : _event_sources) {
            r_code = (RC_OK == it.second->remove_the_observer(i_observer) ? RC_OK : r_code);
        }
    }

    return r_code;
}

int64_t change_buffer_size(int64_t i_size)
{
    static const int64_t page_size = sys_page_size();

    if(i_size > BPF_BUFF_MAX_MEM_SIZE) {
        i_size = BPF_BUFF_MAX_MEM_SIZE;
    }

    /*
     * 2025-8-8：perf_buffer__new()函数需要指定一个缓冲区的大小，这个大小测试后发现需要保证是2^N。
     *     手册并没有明确这个参数要求，参见：https://libbpf.readthedocs.io/en/latest/api.html，搜索函数名
     * ————by OH-Coffee
     */

    return (i_size / (page_size <= 0 ? 4096 : page_size) / 8);
}

} // namespace bpf

