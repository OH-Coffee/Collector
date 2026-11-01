
#ifndef leaky_bucker_h
#define leaky_bucker_h

#include <cstdint>

#include <algorithm>
#include <atomic>

#include <sys/time.h>

class SpeedGovernor
{
public:
    SpeedGovernor(size_t i_size, size_t i_rate) : _size(i_size), _rate(i_rate) {
        _time   = time(nullptr);
        _cursor = i_size;
    }
public:
    bool push(size_t i_num) {
        return (update(), (_cursor + i_num) <= _size) ? (_cursor += i_num) : false;
    }
public:
    inline size_t get_size() { return _size; }
    inline size_t get_rate() { return _rate; }
public:
    inline void   set_size(size_t i_size) { _size = i_size; }
    inline void   set_rate(size_t i_rate) { _rate = i_rate; }
private:
    void update() {
        struct timeval tv; gettimeofday(&tv, nullptr);
        
        const size_t consumption = ((tv.tv_sec - _time) * _rate);
        if(consumption > 0) {
            _time   = tv.tv_sec;
            _cursor = (consumption >= _cursor) ? 0 : (_cursor - consumption);
        }
    }
private:
    std::atomic<size_t> _size;
    std::atomic<size_t> _rate;
private:
    std::atomic<time_t> _time;
    std::atomic<size_t> _cursor;
};


#endif /* leaky_bucker_h */
