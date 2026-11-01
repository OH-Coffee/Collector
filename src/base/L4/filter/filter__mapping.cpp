
#include "file/file_api.hpp"
#include "filter/filter__mapping.hpp"

#define NEW_EXTRACTOR(i_expr) [](const StructSet &i_data) { return i_expr; }

namespace {
    static ProcInfoPtr      g_null_process_info = std::make_shared<ProcInfo>();
    static SCHEMA           g_null_cardinal_msg;
}

namespace filter {

StructSet::StructSet()
{
    proc  = g_null_process_info;
    pproc = g_null_process_info;
    msg   = &g_null_cardinal_msg;
}

static const std::map<FeildKey, FeildExtractor> g_field_map =
{
    { { OID__PROCESS, FID_PROC__PID             }, NEW_EXTRACTOR( std::to_string(i_data.proc->pid)      ) },
    { { OID__PROCESS, FID_PROC__NAME            }, NEW_EXTRACTOR( i_data.proc->name;                    ) },
    { { OID__PROCESS, FID_PROC__PPID            }, NEW_EXTRACTOR( std::to_string(i_data.proc->ppid)     ) },
    { { OID__PROCESS, FID_PROC__WORK_DIR        }, NEW_EXTRACTOR( i_data.proc->work_dir                 ) },
    { { OID__PROCESS, FID_PROC__PATH            }, NEW_EXTRACTOR( i_data.proc->path                     ) },
    { { OID__PROCESS, FID_PROC__ARGS            }, NEW_EXTRACTOR( i_data.proc->args                     ) },
    { { OID__PROCESS, FID_PROC__UID             }, NEW_EXTRACTOR( std::to_string(i_data.proc->uid)      ) },
    { { OID__PROCESS, FID_PROC__SID             }, NEW_EXTRACTOR( std::to_string(i_data.proc->sid)      ) },
    { { OID__PROCESS, FID_PROC__GID             }, NEW_EXTRACTOR( std::to_string(i_data.proc->pgid)     ) },
    
    { { OID__PROCESS_FILE, FID_FILE__NAME       }, NEW_EXTRACTOR( file_name(i_data.proc->path)  ) },
    { { OID__PROCESS_FILE, FID_FILE__PATH       }, NEW_EXTRACTOR( i_data.proc->path             ) },
    
    { { OID__PPROCESS, FID_PROC__PID            }, NEW_EXTRACTOR( std::to_string(i_data.pproc->pid)      ) },
    { { OID__PPROCESS, FID_PROC__NAME           }, NEW_EXTRACTOR( i_data.pproc->name;                    ) },
    { { OID__PPROCESS, FID_PROC__PPID           }, NEW_EXTRACTOR( std::to_string(i_data.pproc->ppid)     ) },
    { { OID__PPROCESS, FID_PROC__WORK_DIR       }, NEW_EXTRACTOR( i_data.pproc->work_dir                 ) },
    { { OID__PPROCESS, FID_PROC__PATH           }, NEW_EXTRACTOR( i_data.pproc->path                     ) },
    { { OID__PPROCESS, FID_PROC__ARGS           }, NEW_EXTRACTOR( i_data.pproc->args                     ) },
    { { OID__PPROCESS, FID_PROC__UID            }, NEW_EXTRACTOR( std::to_string(i_data.pproc->uid)      ) },
    { { OID__PPROCESS, FID_PROC__SID            }, NEW_EXTRACTOR( std::to_string(i_data.pproc->sid)      ) },
    { { OID__PPROCESS, FID_PROC__GID            }, NEW_EXTRACTOR( std::to_string(i_data.pproc->pgid)     ) },
    
    { { OID__PPROCESS_FILE, FID_FILE__NAME      }, NEW_EXTRACTOR( file_name(i_data.pproc->path) ) },
    { { OID__PPROCESS_FILE, FID_FILE__PATH      }, NEW_EXTRACTOR( i_data.pproc->path            ) },
    
    { { OID__NET_CONNECT, FID_NET__PROTOCOL     }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.connect.protocol)     ) },
    { { OID__NET_CONNECT, FID_NET__IS_IPV6      }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.connect.is_ipv6)      ) },
    { { OID__NET_CONNECT, FID_NET__IS_INBOUND   }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.connect.is_inbound)   ) },
    { { OID__NET_CONNECT, FID_NET__SOURCE_PORT  }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.connect.src_port)     ) },
    { { OID__NET_CONNECT, FID_NET__SOURCE_IP    }, NEW_EXTRACTOR( (char *)i_data.msg->Body.connect.src_addr             ) },
    { { OID__NET_CONNECT, FID_NET__DEST_PORT    }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.connect.dst_port)     ) },
    { { OID__NET_CONNECT, FID_NET__DEST_IP      }, NEW_EXTRACTOR( (char *)i_data.msg->Body.connect.dst_addr             ) },
    
    { { OID__FILE, FID_FILE__NAME               }, NEW_EXTRACTOR( file_name((char *)i_data.msg->Body.open.path)       ) },
    { { OID__FILE, FID_FILE__PATH               }, NEW_EXTRACTOR( (char *)i_data.msg->Body.open.path                  ) },
    
    { { OID__NEW_FILE, FID_FILE__NAME           }, NEW_EXTRACTOR( file_name((char *)i_data.msg->Body.rename.target_path) ) },
    { { OID__NEW_FILE, FID_FILE__PATH           }, NEW_EXTRACTOR( (char *)i_data.msg->Body.rename.target_path            ) },
    
    { { OID__NET_DNS, FID_DNS__DOMAIN           }, NEW_EXTRACTOR( (char *)i_data.msg->Body.dns.query_name            ) },
    { { OID__NET_DNS, FID_DNS__QUERY_STATUS     }, NEW_EXTRACTOR( std::to_string(i_data.msg->Body.dns.query_result)  ) },
    { { OID__NET_DNS, FID_DNS__ANSWER           }, NEW_EXTRACTOR( (char *)i_data.msg->Body.dns.answers              ) },
    { { OID__NET_DNS, FID_DNS__ANSWER_TYPE1     }, NEW_EXTRACTOR( "" ) },
    { { OID__NET_DNS, FID_DNS__ANSWER_TYPE5     }, NEW_EXTRACTOR( "" ) },
};
    
const FeildExtractor& new_extractor(const rule::ConditionPtr &i_condtion)
{
    static const FeildExtractor null_extractor = [](const StructSet &i_data){ return ""; };
    
    const auto &it = g_field_map.find( { i_condtion->object, i_condtion->feild } );
    if(it != g_field_map.end()) {
        return it->second;
    }
    
    return null_extractor;
}

    
}

#undef NEW_EXTRACTOR
