
#include "collector.hpp"

static void sig_handler(int i_sig, siginfo_t* info, void* context)
{
    Collector::ins().call_stop(i_sig);
}

ret_code Collector::init_sig_handler()
{
    signal(SIGPIPE,     SIG_IGN);       // 管道错误
    signal(SIGHUP,      SIG_IGN);       // 终端断开

    struct sigaction sa;
    sa.sa_sigaction = sig_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    /* 以下信号的默认行为是进程终止 */
    sigaction(SIGBUS,   &sa, nullptr);  // 总线错误
    sigaction(SIGFPE,   &sa, nullptr);  // 运算错误
    sigaction(SIGILL,   &sa, nullptr);  // 指令错误
    sigaction(SIGINT,   &sa, nullptr);  // Ctrl C
    sigaction(SIGABRT,  &sa, nullptr);
    sigaction(SIGTERM,  &sa, nullptr);
    sigaction(SIGUSR1,  &sa, nullptr);
    sigaction(SIGUSR2,  &sa, nullptr);
    sigaction(SIGSTKSZ, &sa, nullptr);  // 协处理栈错误

    /* 以下信号的默认行为是coredump */
    sigaction(SIGSYS,   &sa, nullptr);  // 系统调用错误
    sigaction(SIGXCPU,  &sa, nullptr);  // 调度时间超限
    sigaction(SIGXFSZ,  &sa, nullptr);  // 文件过大
    sigaction(SIGTRAP,  &sa, nullptr);  // 调试跟踪
    sigaction(SIGQUIT,  &sa, nullptr);  // 终端退出
    sigaction(SIGSEGV,  &sa, nullptr);  // 段错误

    LOG_TRACE("[Collector]\t[YES] 信号处理器注册完成");

    return RC_OK;
}

ret_code Collector::init()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Collector] 初始化开始");
    do {
        if(RC_OK != (r_code = init_sig_handler()))  { break; }
        // if(RC_OK != (r_code = init_stdio()))        { break; }
    } while (false);
    LOG_INFO("[Collector] 初始化完成，错误码：%d", r_code);
    
    return r_code;
}

ret_code Collector::start()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Collector] 开始启动");
    do {
        if(RC_OK != _controller.init()) { break; }

        _controller.start();    // 陷入循环
        _controller.stop();
        
        r_code = _controller.uninit();
    } while (false);
    LOG_INFO("[Collector] 启动中断，错误码：%d", r_code);
    
    return r_code;
}

ret_code Collector::stop()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Collector] 停止开始");
    do {
        r_code = call_stop();
    } while (false);
    LOG_INFO("[Collector] 停止完成，错误码：%d", r_code);
    
    return r_code;
}

ret_code Collector::call_stop(int i_reason)
{
    static bool is_logout = false;

    if(!is_logout) {
        is_logout = true;
        LOG_WARN("");
        LOG_WARN("[Collector]\t[XXX] 收到信号：%d (%s) -> 程序即将结束运行；错误码：%d", i_reason, strsignal(i_reason), RC_EXIT);
        LOG_WARN("");
    }
    
    this->_controller.exit();
    
    return RC_OK;
}

ret_code Collector::uninit()
{
    ret_code r_code = RC_UNKNOWN;
    
    LOG_INFO("[Collector] 反初始化开始");
    do {
        
    } while (false);
    LOG_INFO("[Collector] 反初始化完成，错误码：%d", r_code);
    
    return r_code;
}

