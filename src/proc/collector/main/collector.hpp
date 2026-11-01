
#ifndef Collector_hpp
#define Collector_hpp

#include "pch.hpp"
#include "controller/controller.hpp"

class Collector final
{
public:
    static Collector& ins() {
        static Collector collector; return collector;
    }
private:
    Collector() { _pid = getpid(); }
public:
    ~Collector()  = default;
public:
    Collector(const Collector &)                = delete;
    Collector(const Collector &&)               = delete;
    Collector& operator=(const Collector &)     = delete;
    Collector& operator=(const Collector &&)    = delete;
public:
    ret_code init();
private:
    ret_code init_sig_handler();
    ret_code init_stdio();
    ret_code init_proc_limit();
    ret_code init_proc_info();
public:
    ret_code start();
public:
    ret_code stop();
    ret_code call_stop(int i_reason = SIGUSR1);

public:
    ret_code uninit();
private:
    pid_t   _pid;
private:
    Controller _controller;
private:
#if defined(OS_MACOS)
    dispatch_source_t _dispatch_sources[128];
#endif
};


#endif /* Collector_hpp */

