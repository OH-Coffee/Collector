
#include "thread_pool.hpp"

#include <system_error>

namespace thread {
    
    
ThreadPool::ThreadPool()
{
    _need_stop = false;
}
    
ThreadPool::~ThreadPool()
{
    stop();
}
    
ret_code ThreadPool::init()
{
    return RC_OK;
}

ret_code ThreadPool::start()
{
    std::lock_guard<std::mutex> lifecycle_lock(_lifecycle_mutex);
    if(_need_stop) { return RC_THREAD_POLL_STOP; }
    if(_started) { return RC_OK; }

    _started = true;
    try {
        for(size_t i = 0; i < _max_thread_size; ++i) {
            _worker_threads.emplace_back([this]() {
                while(true) {
                    std::unique_lock<std::mutex> lock(_task_queue_mutex);

                    _task_queue_condition.wait(lock, [this]() {
                        return _need_stop || (!_need_pause && !_task_queue.empty());
                    });
                    if(_need_stop) { break; }

                    std::function<void()> task = std::move(_task_queue.front());
                    _task_queue.pop();
                    --_cur_queue_size;

                    lock.unlock();
                    
                    on_task_begin();
                    task();
                    on_task_finish();
                }
            });
        }
    } catch(const std::system_error&) {
        _need_stop = true;
        _task_queue_condition.notify_all();
        for(auto& worker_thread : _worker_threads) {
            if(worker_thread.joinable()) { worker_thread.join(); }
        }
        _worker_threads.clear();
        return RC_THREAD_POLL_START_ERR;
    }
    
    return RC_OK;
}

ret_code ThreadPool::stop()
{
    std::lock_guard<std::mutex> lifecycle_lock(_lifecycle_mutex);
    _need_stop = true;
    _task_queue_condition.notify_all();

    for(auto& worker_thread : _worker_threads) {
        if(worker_thread.joinable()) { worker_thread.join(); }
    }
    _worker_threads.clear();

    {
        std::lock_guard<std::mutex> queue_lock(_task_queue_mutex);
        while(!_task_queue.empty()) { _task_queue.pop(); }
        _cur_queue_size = 0;
    }
    
    return RC_OK;
}

ret_code ThreadPool::uninit()
{
    this->stop(); return RC_OK;
}

ret_code ThreadPool::resume()
{
    _need_pause = false;
    _task_queue_condition.notify_all();
    return RC_OK;
}
    
ret_code ThreadPool::on_task_begin()
{
    return RC_NO_IMPL;
}
ret_code ThreadPool::on_task_finish()
{
    return RC_NO_IMPL;
}
    
}
