
#include <thread>

#include <fcntl.h>
#include <sys/fanotify.h>

#include "engines/fanotify/fanotify_engine.hpp"
#include "engines/fanotify/fanotify_event_handler_factory.hpp"
#include "file/file_api.hpp"

static const std::list<std::string> g_fanotify_dirs = {
    "/",
    "/home",
    "/root/",
    "/etc/",
    "/usr/",
    "/mnt/",
    "/opt/",
    "/tmp/",
    "/var/",
    "/bin/",
    "/srv/",
    "/boot/",
    "/dev/",
    "/run/",
    "/media/",
    "/recovery/",
    "/cdrom/",
    "/data/",
    "/.gnupg/",
    "/lost+found/",
    "/redis/",
    "/backup/",
};

FanotifyEngine::FanotifyEngine()
{
    _fd_fanotify = -1;
}

FanotifyEngine::~FanotifyEngine()
{
    if(_fd_fanotify > 0) {
        close(_fd_fanotify); _fd_fanotify = -1;
    }
}

ret_code FanotifyEngine::init()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(_fd_fanotify >= 0) { close(_fd_fanotify); _fd_fanotify = -1; }

        // 创建IO驱动器，为事件源提供驱动能力。
        _io_driver = std::make_shared<IO_DriverEpoll>();
        if(nullptr == _io_driver) {
            r_code = RC_IO_DRIVER_CREATE_ERR; break;
        }

        _fd_fanotify = fanotify_init(
            FAN_CLOEXEC        |    // 子进程自动关闭文件描述符
            FAN_CLASS_NOTIF    |    // 仅关心文件变化，不关心内容
            FAN_NONBLOCK            // 设置fanotify_init()返回的fd为非阻塞模式
            ,
            O_RDONLY           |    // 只读模式
            O_LARGEFILE        |    // 大文件支持
            O_CLOEXEC               // 子进程自动关闭文件描述符
        );
        if(_fd_fanotify < 0) {
            r_code = RC_FANOTIFY_INIT_ERR; break;
        }

        // IO驱动器初始化
        if(RC_OK != (r_code = _io_driver->init()))                      { break; }
        if(RC_OK != (r_code = _io_driver->insert(_fd_fanotify, this)))  { break; }
    } while (false);

    return r_code;
}

ret_code FanotifyEngine::start()
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

ret_code FanotifyEngine::stop()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->_io_driver->stop();
    } while(false);

    return r_code;
}

ret_code FanotifyEngine::uninit()
{
    ret_code r_code = RC_UNKNOWN;

    do {
        r_code = this->_io_driver->uninit();

        if(_fd_fanotify > 0) {
            close(_fd_fanotify); _fd_fanotify = -1;
        }
    } while(false);

    return r_code;
}

ret_code FanotifyEngine::subscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        FanotifyEventHandlerPtr fanotify_handler = FanotifyEventHandlerFactory::create(i_event);
        if(nullptr == fanotify_handler) {
            r_code = RC_UNCONCERNED; break;
        }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            auto it = _event_handlers.find(fanotify_handler->event());
            if(_event_handlers.end() != it) {
                fanotify_handler = it->second;
            } else {
                _event_handlers[fanotify_handler->event()] = fanotify_handler;
            }
        }
        if(nullptr == fanotify_handler) { r_code = RC_FAIL; break; }

        r_code = this->fanotify_add(fanotify_handler->event());
        if(RC_OK == r_code) {
            r_code = fanotify_handler->observer_add(i_observer);
        }
    } while (false);

    return r_code;
}

ret_code FanotifyEngine::unsubscribe(EventType i_event, EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        // 通过此方法来判断事件码是否可被此事件引擎受理
        if(nullptr == FanotifyEventHandlerFactory::create(i_event)) {
            r_code = RC_UNCONCERNED; break;
        }

        FanotifyEventHandlerPtr fanotify_handler = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            auto it = _event_handlers.find(fanotify_handler->event());
            if(_event_handlers.end() != it) {
                fanotify_handler = it->second;
            }
        }
        if(nullptr == fanotify_handler) { r_code = RC_UNCONCERNED; break; }

        r_code = fanotify_handler->observer_del(i_observer);
        if(fanotify_handler->observer_num() <= 0) {
            {
                std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);
                _event_handlers.erase(fanotify_handler->event());
            }
            r_code = this->fanotify_del(fanotify_handler->event());
        }
    } while (false);

    return r_code;
}

ret_code FanotifyEngine::remove_the_event(EventType i_event)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        FanotifyEventHandlerPtr fanotify_handler = FanotifyEventHandlerFactory::create(i_event);
        if(nullptr == fanotify_handler) {
            r_code = RC_UNCONCERNED; break;
        }

        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);
            _event_handlers.erase(fanotify_handler->event());
        }
        r_code = this->fanotify_del(fanotify_handler->event());
    } while (false);

    return r_code;
}
ret_code FanotifyEngine::remove_the_observer(EventObserver *i_observer)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == i_observer) { r_code = RC_OK; break; }

        FanotifyEventHandlerPtr fanotify_handler = nullptr;
        {
            std::lock_guard<std::mutex> mutex_guard(_mutex_for_event_handlers);

            for(auto it = _event_handlers.begin(); it != _event_handlers.end(); ) {
                if(nullptr == (fanotify_handler = it->second)) { ++it; continue; }

                if(RC_OK == fanotify_handler->observer_del(i_observer)) {
                    r_code = RC_OK;
                }

                if(fanotify_handler->observer_num() > 0) {
                    ++it;
                } else {
                    it     = _event_handlers.erase(it);
                    r_code = this->fanotify_del(fanotify_handler->event());
                }
            }
        }
    } while (false);

    return r_code;
}

ret_code FanotifyEngine::fanotify_add(FanotifyEventType i_event) const
{
    ret_code r_code = RC_UNKNOWN;
    do {
        for(const auto &path : g_fanotify_dirs) {
            if(RC_OK != file_is_exist(path)) { continue; }

            uint32_t fan_mark_flag = FAN_MARK_ADD | FAN_MARK_MOUNT;
            uint64_t fan_mark_mask = i_event;
            int      fan_mark_dfd  = 0;

            int err_code = fanotify_mark(_fd_fanotify, fan_mark_flag, fan_mark_mask, fan_mark_dfd, path.c_str());
            if(err_code == 0) {
                r_code = RC_OK;
            } else {
                r_code = RC_FANOTIFY_MARK_ERR; break;
            }
        }
    } while (false);

    return r_code;
}

ret_code FanotifyEngine::fanotify_del(FanotifyEventType i_event) const
{
    ret_code r_code = RC_UNKNOWN;

    do {
        static const std::list<std::string> paths = { "/", "/home" };

        for(const auto &path : paths) {
            uint32_t fan_mark_flag = FAN_MARK_REMOVE | FAN_MARK_MOUNT;
            uint64_t fan_mark_mask = i_event;
            int      fan_mark_dfd  = 0;

            int err_code = fanotify_mark(_fd_fanotify, fan_mark_flag, fan_mark_mask, fan_mark_dfd, path.c_str());
            if(err_code == 0) {
                r_code = RC_OK;
            } else {
                r_code = RC_FANOTIFY_MARK_ERR; break;
            }
        }
    } while (false);

    return r_code;
}
