
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/fcntl.h>

#include <pthread.h>

#include "process/proc_control.hpp"

void proc_close_fd(const int i_start_fd)
{
    const auto max_fd_num = sysconf(_SC_OPEN_MAX);

    for (int fd = i_start_fd; fd < max_fd_num; fd++) {
        if (fcntl(fd, F_GETFD) != -1) { ::close(fd); }
    }
}

bool proc_not_alive(pid_t i_pid)
{
    if(i_pid <= 0) { return true; }
    
    if( (kill(i_pid, 0) < 0) && (errno == ESRCH) ) {
        return true;    /* 注意：errno非线程安全 */
    }
    
    return false;
}

bool proc_kill(pid_t i_pid, int i_sig)
{
    if(proc_not_alive(i_pid)) { return true; }
    
    return (kill(i_pid, i_sig) == 0 ? true : false);
}

bool proc_softkill(pid_t i_pid)
{
    for(int i=5; i>=0; --i) {
        if(proc_not_alive(i_pid)) {
            return true;
        } else {
            kill(i_pid, (i == 1 ? SIGKILL : SIGTERM)); sleep(1);
        }
    }
    return false;
}

bool proc_killwait(pid_t i_pid)
{
    for(int i=5; i>=0; --i) {
        if(proc_not_alive(i_pid)) {
            waitpid(i_pid, nullptr, WNOHANG); return true;
        } else {
            kill(i_pid, (i == 1 ? SIGKILL : SIGTERM)); sleep(1);
        }
    }
    return false;
}

pid_t proc_new_subproc(const char *i_exe_path, char * const i_args[])
{
    pid_t r_fork_pid = fork();
    
    if (r_fork_pid > 0) {
        return r_fork_pid;
    } else if (r_fork_pid == 0) {
        setsid();
        umask(0);
        sleep(1);
        
        if(0 != access(i_exe_path, F_OK)) { exit(-1); }
        
        sigset_t sig_mask;
        sigemptyset(&sig_mask);
        sigprocmask(SIG_SETMASK, &sig_mask, nullptr);
        
        struct rlimit rl{ 0, 0 };
        getrlimit(RLIMIT_NOFILE, &rl);
        rlim_t fd_max = (rl.rlim_max < 1024) ? rl.rlim_max : 1024;

        for(int fd = 0; fd < (int)fd_max; fd++) { close(fd); }
        
        chmod(i_exe_path, S_IRUSR | S_IWUSR | S_IXUSR);
        
        execv(i_exe_path, i_args); exit(0);
    }
    
    return 0;
}

bool proc_new_daemon(const char *i_exe_path, char* const i_args[])
{
    pid_t r_fork_pid = fork();
    
    if (r_fork_pid > 0) {
        /* 回收子进程，避免其成为Z进程 */
        for(int i=0; i<5; i++) {
            int r_code = waitpid(r_fork_pid, nullptr, WUNTRACED|WNOHANG);
            
            if(r_code <= -1) { return true; }  // 子进程已退出
            if(r_code ==  0) { sleep(1);    }  // 子进程仍存在
        }
        
        proc_kill(r_fork_pid, SIGKILL); return true;
    } else if (r_fork_pid == 0) {
        setsid();
        umask(0);
        
        sigset_t sig_mask;
        sigemptyset(&sig_mask);
        sigprocmask(SIG_SETMASK, &sig_mask, nullptr);
        
        struct rlimit rl;
        getrlimit(RLIMIT_NOFILE, &rl);
        rlim_t fd_max = (rl.rlim_max < 1024) ? rl.rlim_max : 1024;

        for(int fd = 0; fd < (int)fd_max; fd++) { close(fd); }
        
        proc_new_subproc(i_exe_path, i_args);
        
        exit(0); /* 退出后，1号进程接管proc_new_subproc()创建的子进程，使该子进程成为后台进程 */
    }
    
    return false;
}

static std::string read_all(const int i_fd, const size_t i_max_size, int &o_err)
{
    std::string ret_str;

    do {
        if (i_fd < 0 || i_max_size <= 0) { break; }

        auto io_size   = static_cast<ssize_t>(0);
        auto remainder = static_cast<ssize_t>(i_max_size);

        constexpr int buf_len = 1024; char buf[buf_len + 1]; bzero(buf, buf_len + 1);
        while((remainder -= io_size) > 0) {
            if((io_size = ::read(i_fd, buf, buf_len)) <= 0) {
                break;
            }
            ret_str.append(buf); bzero(buf, buf_len);
        }
        o_err = (io_size < 0 && errno != EAGAIN) ? -errno : 0;
    } while (false);

    return ret_str;
}

uint64_t proc_exec_cmd(const std::string &i_cmd,
                       std::string &o_std_out,
                       std::string &o_std_err,
                       const size_t i_max_out,
                       const time_t i_timeout,
                       const bool i_force_kill)
{
    uint64_t ret_code = PROC_EXEC_OK;

    int pipe_fd_set[2][2]; bzero(pipe_fd_set, sizeof(pipe_fd_set));

    if(0 != pipe(pipe_fd_set[0]) || 0 != pipe(pipe_fd_set[1])) {
        ret_code |= PROC_EXEC_PIPE_ERR;
    }

    const int &std_out_read_end  = pipe_fd_set[0][0];     // 标准输出读端
    const int &std_out_write_end = pipe_fd_set[0][1];     // 标准输出写端
    const int &std_err_read_end  = pipe_fd_set[1][0];     // 标准错误读端
    const int &std_err_write_end = pipe_fd_set[1][1];     // 标准错误写端

    fcntl(std_out_read_end, F_SETFL, O_NONBLOCK);
    fcntl(std_err_read_end, F_SETFL, O_NONBLOCK);

    fflush(stdout);
    fflush(stderr);

    const signal_handler_t default_sig_handler = signal(SIGCHLD, SIG_DFL);

    const pid_t pid_subproc = fork(); // fork()创建子进程用于执行用户命令
    if (pid_subproc == 0) {
        dup2(std_out_write_end, STDOUT_FILENO);
        dup2(std_err_write_end, STDERR_FILENO);

        proc_close_fd(3);

        sigset_t mask; sigfillset(&mask); pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);

        const std::string path    = "PATH=$PATH:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin:./;";
        const std::string ld_path = "LD_LIBRARY_PATH=/lib:/lib64:/usr/lib:/usr/lib64:/usr/local/lib/:/usr/local/lib64/:./;";
        const std::string exec_cmd = path + ld_path + i_cmd;

        exit( execl("/bin/bash", "bash", "-c", exec_cmd.c_str(), nullptr) );
    } else if (pid_subproc > 0) {
        const time_t end_time = time(nullptr) + i_timeout + 1;

        int kill_signal    = 0;
        int kill_counter   = 0;

        int r_std_out_read = 0;
        int r_std_err_read = 0;

        int subproc_exit_status = 0;

        pid_t r_waitpid = 0;
        do {
            usleep(1000); // waitpid()的重复间隔

            r_waitpid = waitpid(pid_subproc, &subproc_exit_status, WNOHANG);

            if ((time(nullptr) >= end_time) || (o_std_out.length() >= i_max_out)) {
                if (!i_force_kill) {
                    kill_signal = SIGINT;
                } else { switch (kill_counter++) {
                        case 0:  { kill_signal = SIGTERM; } break;
                        case 1:  { kill_signal = SIGINT;  } break;
                        default: { kill_signal = SIGKILL; } break;
                    } }

                kill(pid_subproc, kill_signal); sleep(1); continue;
            }

            o_std_out.append( read_all(std_out_read_end, i_max_out - o_std_out.length(), r_std_out_read) );
            o_std_err.append( read_all(std_err_read_end, i_max_out - o_std_err.length(), r_std_err_read) );
        } while (r_waitpid == 0);

        if (r_waitpid != pid_subproc) {
            ret_code |= PROC_EXEC_WAIT_ERR;
        }

        if (WIFEXITED(subproc_exit_status)) {
            ret_code = WEXITSTATUS(subproc_exit_status);
        } else {
            ret_code |= (kill_signal == 0 ? PROC_EXEC_MAY_CRASH : PROC_EXEC_TIMEOUT);
        }

        ret_code |= (r_std_out_read < 0 ? PROC_EXEC_READ_STD_OUT_FAILED : ret_code);
        ret_code |= (r_std_err_read < 0 ? PROC_EXEC_READ_STD_ERR_FAILED : ret_code);
    } else {
        ret_code |= PROC_EXEC_FORK_ERR;
    }

    ::close(std_out_read_end);
    ::close(std_err_read_end);
    ::close(std_out_write_end);
    ::close(std_err_write_end);

    signal(SIGCHLD, default_sig_handler);

    return ret_code;
}
