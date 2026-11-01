
#include <cstring>

#include <string>

#include <memory.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include <libproc.h>
#include <mach/mach.h>
#include <mach/thread_act.h>

#include "process/proc_info.hpp"
#include "string/string_api.hpp"
#include "system/sys_info.hpp"


int proc_all_pids(pid_t o_pid_list[], int i_list_size)
{
    return proc_listpids(PROC_ALL_PIDS, 0, o_pid_list, i_list_size);
}

int proc_pids_by_name(const char *i_name, std::vector<pid_t> &o_pids, bool i_ignore_case)
{
    int ret_counter = 0;
    
    do {
        if(!i_name) { break; }
        
        const int pid_num   = 10000;
        pid_t pids[pid_num] = {0};
        
        const size_t len = strlen(i_name);
        
        if(proc_all_pids(pids, pid_num) > 0) {
            for(pid_t pid : pids) {
                bool match = false;
                if(!i_ignore_case) {
                    if(0 == strncmp(proc_name(pid).c_str(), i_name, len))     { match = true; }
                } else {
                    if(0 == strncasecmp(proc_name(pid).c_str(), i_name, len)) { match = true; }
                }
                if(match) { o_pids.push_back(pid); ++ret_counter; }
            }
        }
    } while(0);
    
    return ret_counter;
}

int proc_pids_by_path(const char *i_path, std::vector<pid_t> &o_pids, bool i_ignore_case)
{
    int ret_counter = 0;
    
    do {
        if(!i_path) { break; }
        
        const int pid_num   = 10000;
        pid_t pids[pid_num] = {0};
        
        const size_t len = strlen(i_path);
        
        if(proc_all_pids(pids, pid_num) > 0) {
            for(pid_t pid : pids) {
                bool match = false;
                if(!i_ignore_case) {
                    if(0 == strncmp(proc_path(pid).c_str(), i_path, len))     { match = true; }
                } else {
                    if(0 == strncasecmp(proc_path(pid).c_str(), i_path, len)) { match = true; }
                }
                if(match) { o_pids.push_back(pid); ++ret_counter; }
            }
        }
    } while(0);
    
    return ret_counter;
}

char* proc_name(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    if(i_buf_len <= 0)   { return nullptr; }
    if(nullptr == o_buf) { return nullptr; }
    
    if(proc_name(i_pid, o_buf, i_buf_len) > 0) {
        return o_buf;
    }
    
    return nullptr;
}

std::string proc_name(pid_t i_pid)
{
    char buf[256]; bzero(buf, sizeof(buf));
    
    if(proc_name(i_pid, buf, sizeof(buf)) > 0) {
        return buf;
    }
    
    return "";
}

char * proc_path(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    if(i_buf_len <= 0)   { return nullptr; }
    if(nullptr == o_buf) { return nullptr; }
    
    if(proc_pidpath(i_pid, o_buf, i_buf_len) > 0) {
        return o_buf;
    }
    
    return nullptr;
}

std::string proc_path(pid_t i_pid)
{
    char buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
    
    if(proc_pidpath(i_pid, buf, sizeof(buf)) > 0) {
        return buf;
    }
    
    return "";
}

char * proc_work_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    if(i_buf_len <= 0)   { return nullptr; }
    if(nullptr == o_buf) { return nullptr; }
    
    struct proc_vnodepathinfo vpi = {};
    
    if(proc_pidinfo(i_pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi)) > 0) {
        return (char *)memcpy(o_buf, vpi.pvi_cdir.vip_path, MIN(i_buf_len, sizeof(vpi.pvi_cdir.vip_path)));
    }
    
    return nullptr;
}
std::string proc_work_dir(pid_t i_pid)
{
    char buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
    
    return proc_work_dir(i_pid, sizeof(buf), buf) ? buf : "";
}

char * proc_root_dir(pid_t i_pid, uint32_t i_buf_len, char *o_buf)
{
    if(i_buf_len <= 0)   { return nullptr; }
    if(nullptr == o_buf) { return nullptr; }
    
    struct proc_vnodepathinfo vpi = {};
    
    if(proc_pidinfo(i_pid, PROC_PIDVNODEPATHINFO, 0, &vpi, sizeof(vpi)) > 0) {
        return (char *)memcpy(o_buf, vpi.pvi_rdir.vip_path, MIN(i_buf_len, sizeof(vpi.pvi_rdir.vip_path)));
    }
    
    return nullptr;
}
std::string proc_root_dir(pid_t i_pid)
{
    char buf[PROC_PIDPATHINFO_MAXSIZE] = {0};
    
    return proc_root_dir(i_pid, sizeof(buf), buf) ? buf : "";
}

std::vector<std::string> proc_stat(pid_t i_pid)
{
    return {};
}

std::vector<std::string> proc_args_info(pid_t i_pid, ProcArgsInfo i_type)
{
    std::vector<std::string> r_args;
    
    do {
        std::string arg_str;
        
        size_t str_len = 0;
        int mib[] = {CTL_KERN, KERN_PROCARGS2, i_pid};
        if (sysctl(mib, 3, nullptr, &str_len, nullptr, 0) != 0) { break; }
        
        arg_str.resize(++str_len, 0);
        
        if(0 != sysctl(mib, 3, arg_str.data(), &str_len, nullptr, 0)) {
            break;
        }
        
        int argc = *(int *)arg_str.c_str();
        if(argc <= 0 ) { break; }
        
        if(static_cast<uint32_t>(i_type) & PROC_ARG_ARGC) { r_args.push_back(std::to_string(argc)); }
        
        const char *cursor_ptr = arg_str.c_str() + sizeof(argc);
        const char *cursor_end = arg_str.c_str() + arg_str.size();
        
        if(static_cast<uint32_t>(i_type) & PROC_ARG_PATH) { r_args.push_back(cursor_ptr); }
        
        if((cursor_ptr = strchr(cursor_ptr, '\0')) == nullptr)  { break; }
        
        while('\0' == *(++cursor_ptr)) { ; }
        
        for(int i = 0; i < argc; ++i) {
            if(static_cast<uint32_t>(i_type) & PROC_ARG_ARGV) { r_args.push_back(cursor_ptr); }
            
            if(!(cursor_ptr = strchr(cursor_ptr, '\0'))) { cursor_ptr = nullptr; break; }
            if((++cursor_ptr) >= cursor_end)             { cursor_ptr = nullptr; break; }
        }
        
        if(nullptr == cursor_ptr) { break; }
        
        while(('\0' != *cursor_ptr) && ('\0' != *(cursor_ptr + 1))) {
            if(static_cast<uint32_t>(i_type) & PROC_ARG_ENVS) { r_args.push_back(cursor_ptr); }
            
            if(!(cursor_ptr = strchr(cursor_ptr, '\0'))) { break; }
            if((++cursor_ptr) >= cursor_end)             { break; }
        }
    } while (0);
    
    return r_args;
}

std::string proc_args(pid_t i_pid)
{
    return string_joint(proc_args_info(i_pid, PROC_ARG_ARGV));
}

std::string proc_envs(pid_t i_pid)
{
    return string_joint(proc_args_info(i_pid, PROC_ARG_ENVS));
}

std::string proc_cmdline(pid_t i_pid)
{
    return string_joint(proc_args_info(i_pid, PROC_ARG_ARGV));
}

bool proc_threads(pid_t i_pid, std::vector<uint64_t> &o_tid)
{
    bool ret_bool = false;
    
    do {
        kern_return_t   r_code;
        
        mach_port_t task;
        r_code = task_for_pid(mach_task_self(), i_pid, &task);
        if(r_code != KERN_SUCCESS) { break; }
        
        thread_act_array_t      thread_list;
        mach_msg_type_number_t  thread_count;
        r_code = task_threads(task, &thread_list, &thread_count);
        if(r_code != KERN_SUCCESS) { break; }
        
        int tid_counter = 0;
        
        thread_identifier_info_data_t thread_id_info;
        mach_msg_type_number_t        thread_id_info_count = THREAD_IDENTIFIER_INFO_COUNT;
        
        for (mach_msg_type_number_t i = 0; i < thread_count; ++i) {
            r_code = thread_info(thread_list[i], THREAD_IDENTIFIER_INFO, (thread_info_t)&thread_id_info, &thread_id_info_count);
            if(r_code == KERN_SUCCESS) {
                ++tid_counter; o_tid.push_back(thread_list[i]);
            }
        }
        
        r_code = vm_deallocate(mach_task_self(), (vm_address_t)thread_list, thread_count * sizeof(thread_t));
        if(r_code != KERN_SUCCESS) { break; }
        
        if(tid_counter == thread_count) { ret_bool = true; }
    } while (false);
    
    return ret_bool;
}

bool proc_threads_cpu(pid_t i_pid, std::map<uint64_t, int> &o_info)
{
    bool ret_bool = false;
    
    do {
        kern_return_t   r_code;
        
        mach_port_t task;
        r_code = task_for_pid(mach_task_self(), i_pid, &task);
        if(r_code != KERN_SUCCESS) { break; }
        
        thread_act_array_t      thread_list;
        mach_msg_type_number_t  thread_count;
        r_code = task_threads(task, &thread_list, &thread_count);
        if(r_code != KERN_SUCCESS) { break; }
        
        int tid_counter = 0;
        
        thread_identifier_info_data_t thread_id_info;
        mach_msg_type_number_t        thread_id_info_count = THREAD_IDENTIFIER_INFO_COUNT;
        
        thread_basic_info_data_t      thread_bs_info;
        mach_msg_type_number_t        thread_bs_info_count = THREAD_BASIC_INFO_COUNT;
        
        for (mach_msg_type_number_t i = 0; i < thread_count; ++i) {
            r_code = thread_info(thread_list[i], THREAD_IDENTIFIER_INFO, (thread_info_t)&thread_id_info, &thread_id_info_count);
            if(r_code != KERN_SUCCESS) { continue; }
            
            r_code = thread_info(thread_list[i], THREAD_BASIC_INFO,      (thread_info_t)&thread_bs_info, &thread_bs_info_count);
            if(r_code != KERN_SUCCESS) { continue; }
            
            o_info[thread_id_info.thread_id] = thread_bs_info.cpu_usage; ++tid_counter;
        }
        
        r_code = vm_deallocate(mach_task_self(), (vm_address_t)thread_list, thread_count * sizeof(thread_t));
        if(r_code != KERN_SUCCESS) { break; }
        
        if(tid_counter == thread_count) { ret_bool = true; }
    } while (false);
    
    return ret_bool;
}

uint64_t proc_cpu_time(pid_t i_pid)
{
    static const uint64_t tick = sys_clock_tick();
    
    if(struct proc_taskinfo info; proc_pidinfo(i_pid, PROC_PIDTASKINFO, 0, &info, sizeof(info)) > 0) {
        return (info.pti_total_user + info.pti_total_system) / tick;
    }
    
    return 0;
}

uint64_t proc_cpu_usage(pid_t i_pid)
{
    uint64_t r_value = 0;
    
    do {
        std::map<uint64_t, int> cpu_info;
        if(!proc_threads_cpu(i_pid, cpu_info)) { break; }
        
        for(const auto &it : cpu_info) {
            r_value += (it.second / 10);
        }
    } while (false);
    
    return r_value;
}

uint64_t proc_opened_fd_cnt(pid_t i_pid)
{
    uint64_t count = 0;
    int res = proc_pidinfo(i_pid, PROC_PIDLISTFDS, 0, NULL, 0);
    if (0 >= res) {
        return count;
    }
    
    struct proc_fdinfo fds[32];
    struct proc_fdinfo *pfds =  NULL;
    if (sizeof(fds) >= res) {
        pfds = fds;
    }
    else {
        pfds = (struct proc_fdinfo *)malloc(res);
        if (NULL == pfds) {
            return count;
        }
    }
    
    // 这个函数很挫，必须调用第二次才能获取到正确的数量，第一次会比预期值大
    res = proc_pidinfo(i_pid, PROC_PIDLISTFDS, 0, pfds, res);
    count = res / PROC_PIDLISTFD_SIZE;
    if (pfds != fds) free(pfds);
    
    return count;
}

uint64_t proc_self_opened_fd_cnt()
{
    uint64_t count = 0;
    DIR *dir = opendir("/dev/fd");
    if (dir) {
        while (readdir(dir) != NULL) {
            count++;
        }
        closedir(dir);
    }
    return count;
}

uint64_t proc_rss_mem(pid_t i_pid)
{
    uint64_t r_value = false;
    
    do {
        proc_taskinfo task_info;
        int r_proc = proc_pidinfo(i_pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));
        if(r_proc < sizeof(task_info)) { break; }
        
        r_value = task_info.pti_resident_size;
    } while (false);
    
    return r_value;
}

uint64_t proc_virtual_mem(pid_t i_pid)
{
    uint64_t r_value = false;
    
    do {
        proc_taskinfo task_info;
        int r_proc = proc_pidinfo(i_pid, PROC_PIDTASKINFO, 0, &task_info, sizeof(task_info));
        if(r_proc < sizeof(task_info)) { break; }
        
        r_value = task_info.pti_virtual_size;
    } while (false);
    
    return r_value;
}


bool proc_audit_token(pid_t i_pid, audit_token_t &o_token)
{
    bool r_bool = true;
    
    do {
        kern_return_t r_kern = 0;
        
        task_name_t task_name;
        r_kern = task_name_for_pid(mach_task_self(), i_pid, &task_name);
        if(0 != r_kern) { r_bool = false; break; }
        
        mach_msg_type_number_t audit_token_len = TASK_AUDIT_TOKEN_COUNT;
        
        r_kern = task_info(task_name, TASK_AUDIT_TOKEN, (task_info_t)&o_token, &audit_token_len);
        if(0 != r_kern) { r_bool = false; break; }
    } while (false);
    
    return r_bool;
}

bool proc_flags_is_set(uint64_t i_flags, uint64_t i_flag_to_check)
{
    return false;
}
