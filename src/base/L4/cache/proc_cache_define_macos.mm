
#include <algorithm>

#include "file/file_api.hpp"
#include "process/proc_info.hpp"
#include "cache/proc_cache_define.hpp"

ret_code ProcCache::init()
{
    ret_code r_code = RC_UNKNOWN;

    this->disable(); // 缓存失能，避免内部、外部调用插入、查找;
    do {
        pid_t pids[3000]; bzero(pids, sizeof(pids));

        // 获取系统所有进程的PID，最大3k个
        int r_size = proc_all_pids(pids, sizeof(pids));
        if(r_size <= 0) { r_code = RC_PROC; break; }

        // 将PID按升序排序
        int pid_num = (r_size / sizeof(pid_t));
        std::sort(pids, pids + pid_num);

        std::lock_guard<std::mutex> mutex_guard(_mutex);

        for(int i = 0; i < pid_num; ++i) {
            pid_t pid = pids[i];
            if(pid <= 0) { continue; }

            // 获取进程audit token
            audit_token_t audit_token;
            if(!proc_audit_token(pid, audit_token)) { continue; }

            // 获取进程BSD信息
            proc_bsdinfo bsd_info;
            int r_proc = proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bsd_info, PROC_PIDTBSDINFO_SIZE);
            if(r_proc <= 0) { continue; }

            // 创建缓存项
            auto result = _cache.emplace(pid, NEW_PROC_CACHE_ITEM());
            if(false == result.second) { continue; }

            ProcCacheKey     key(audit_token);
            ProcCacheItemPtr cache_item = result.first->second;

            // 缓存项初始化
            cache_item->last_time = 0;
            cache_item->last_info = std::make_shared<ProcInfo>();
            cache_item->proc_infos.emplace(key.version, cache_item->last_info);

            ProcInfoPtr proc = cache_item->last_info;

            // 进程信息初始化
            {
                proc->pid  = bsd_info.pbi_pid;
                proc->ppid = bsd_info.pbi_ppid;

                guid_calculate(&audit_token, bsd_info, proc->guid);

                proc->stime.tv_sec  = bsd_info.pbi_start_tvsec;
                proc->stime.tv_usec = bsd_info.pbi_start_tvusec & 0x00000000FFFFFFFF;

                proc->audit_token = audit_token;
                guid_calculate(&audit_token, bsd_info, proc->guid);
            }
            {
                proc->name = proc_name(proc->pid);
                proc->path = proc_path(proc->pid);

                if(proc->name.empty()) {
                    proc->name = file_name(proc->path);
                }

                proc->args = proc_args(proc->pid);
//                proc->envs = proc_envs(proc->pid);
            }
            {
                auto sign_info = file_sign_info(proc->path);
                
                proc->sign_id = sign_info[FSI_TYPE_SIGN_ID];
                proc->team_id = sign_info[FSI_TYPE_TEAM_ID];
            }

            // 获取父进程的GUID
            {
                ProcInfoPtr pproc = nullptr;

                auto cache_item = _cache.find( proc->ppid );
                if(_cache.end() != cache_item) {
                    pproc = cache_item->second->last_info;
                }

                r_code = (pproc != nullptr) ? guid_copy(pproc->guid, proc->pguid) : guid_calculate(proc->ppid, proc->pguid);
                if(RC_OK != r_code) {
                    ;
                }
            }
        }

        r_code = RC_OK;
    } while (false);
    this->enable();  // 缓存使能

    return r_code;
}
