
#ifndef sleep_hpp
#define sleep_hpp

#include "pch.hpp"

class Sleep
{
public:
    ret_code wakeup();
public:
    ret_code sleep(uint64_t     i_time_sec);
    ret_code sleep_ms(uint64_t  i_time_ms);
private:
    std::mutex              _mutex;
    std::condition_variable _condition;
};

#endif /* sleep_hpp */

