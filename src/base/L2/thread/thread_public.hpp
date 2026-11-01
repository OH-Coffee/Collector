
#ifndef thread_public_hpp
#define thread_public_hpp

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "header/errcode.hpp"

namespace thread { // namespace: thread

#define MAX_THREAD_SIZE         10
#define MAX_QUEUE_SIZE          (MAX_THREAD_SIZE * 200)

#define DEFAULT_THREAD_SIZE     5
#define DEFAULT_QUEUE_SIZE      (DEFAULT_THREAD_SIZE * 200)
#define DEFAULT_TASK_INTERVAL   1000

}   // namespace: thread


#endif /* thread_public_hpp */
