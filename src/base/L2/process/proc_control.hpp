
#ifndef proc_ctrl_posix_hpp
#define proc_ctrl_posix_hpp

#include <csignal>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <ctime>
#include <unistd.h>

typedef void (*signal_handler_t)(int);


enum : uint64_t
{
    PROC_EXEC_OK                    = 0,

    PROC_EXEC_PIPE_ERR              = static_cast<uint64_t>(1) << (32 + 1),
    PROC_EXEC_FORK_ERR              = static_cast<uint64_t>(1) << (32 + 2),
    PROC_EXEC_WAIT_ERR              = static_cast<uint64_t>(1) << (32 + 3),

    PROC_EXEC_TIMEOUT               = static_cast<uint64_t>(1) << (32 + 10),
    PROC_EXEC_MAY_CRASH             = static_cast<uint64_t>(1) << (32 + 11),
    PROC_EXEC_READ_STD_OUT_FAILED   = static_cast<uint64_t>(1) << (32 + 12),
    PROC_EXEC_READ_STD_ERR_FAILED   = static_cast<uint64_t>(1) << (32 + 13),
};

void    proc_close_fd(int i_start_fd);

bool    proc_not_alive(pid_t i_pid);
bool    proc_kill(pid_t i_pid, int i_sig);
bool    proc_softkill(pid_t i_pid);
bool    proc_killwait(pid_t i_pid);
pid_t   proc_new_subproc(const char *i_exe_path, char * const i_args[]);
bool    proc_new_daemon(const char *i_exe_path, char * const i_args[]);

uint64_t    proc_exec_cmd(const std::string &i_cmd,
                          std::string &o_std_out,
                          std::string &o_std_err,
                          size_t i_max_out    = 1024,
                          time_t i_timeout    = 3,
                          bool   i_force_kill = false);

#endif /* proc_ctrl_posix_hpp */
