
#ifndef thread_pool_hpp
#define thread_pool_hpp

#include <functional>
#include <queue>
#include <thread>
#include <tuple>
#include <type_traits>
#include <vector>

#include "thread_public.hpp"

namespace thread {

class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();
public:
    ret_code init();
    ret_code start();
    ret_code stop();
    ret_code uninit();
public:
    inline ret_code pause()    { _need_pause = true; return RC_OK; }
    ret_code resume();
public:
    inline ret_code is_full()  { return (_cur_queue_size >= _max_queue_size ? RC_OK : RC_FAIL); }
public:
    inline size_t set_queue_size(size_t i_size)    { return _max_queue_size  = i_size; }
    inline size_t set_task_interval(size_t i_size) { return _task_interval   = i_size; }
public:
    inline size_t get_thread_size()                { return _max_thread_size.load(std::memory_order_relaxed); }
    inline size_t set_thread_size(size_t i_size)   { return _max_thread_size = i_size; }
private:
    ret_code on_task_begin();
    ret_code on_task_finish();
public:
    template<class F, class... Args>
    inline ret_code add_task(F && f, Args && ... args);
private:
    std::atomic_bool    _need_stop{false};
    std::atomic_bool    _need_pause{false};
    bool                _started{false};
private:
    std::atomic_size_t  _cur_queue_size{0};
    std::atomic_size_t  _max_queue_size{DEFAULT_QUEUE_SIZE};
    std::atomic_size_t  _max_thread_size{DEFAULT_THREAD_SIZE};
    size_t              _task_interval{DEFAULT_TASK_INTERVAL};
private:
    std::queue< std::function<void()> > _task_queue;
    std::mutex                          _task_queue_mutex;
    std::condition_variable             _task_queue_condition;
    std::mutex                          _lifecycle_mutex;
    std::vector<std::thread>            _worker_threads;
};

template<class F, class... Args>
ret_code ThreadPool::add_task(F && f, Args && ... args)
{
    std::lock_guard<std::mutex> lock(_task_queue_mutex);
    if(_need_stop) {
        return RC_THREAD_POLL_STOP;
    }

    if(RC_OK != this->is_full()) {
        _task_queue.emplace(
            [i_func = std::decay_t<F>(std::forward<F>(f)),
             i_args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                std::apply([&i_func](auto&... i_bound_args) {
                    std::invoke(std::forward<F>(i_func), std::forward<Args>(i_bound_args)...);
                }, i_args);
            });

        ++_cur_queue_size;
        _task_queue_condition.notify_one(); return RC_OK;
    }

    return RC_THREAD_POLL_TASK_QUEUE_FULL;
}

}

#endif /* thread_pool_hpp */
