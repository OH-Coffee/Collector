
#include "sleep/sleep.hpp"

ret_code Sleep::wakeup()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.notify_all();
    
    return RC_OK;
}

ret_code Sleep::sleep(uint64_t i_time_sec)
{
    return this->sleep_ms(i_time_sec * 1000);
}

ret_code Sleep::sleep_ms(uint64_t i_time_ms)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait_for(lock, std::chrono::milliseconds(i_time_ms));
    
    return RC_OK;
}

