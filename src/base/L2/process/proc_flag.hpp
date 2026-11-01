#ifndef process_proc_flag_hpp
#define process_proc_flag_hpp

// https://www.man7.org/linux/man-pages/man5/proc_pid_stat.5.html
// https://elixir.bootlin.com/linux/v4.19/source/include/linux/sched.h

#ifndef PF_IDLE
    #define PF_IDLE             0x00000002
    #define PF_KTHREAD          0x00200000
    #define PF_RANDOMIZE        0x00400000
#endif

#endif // process_proc_flag_hpp