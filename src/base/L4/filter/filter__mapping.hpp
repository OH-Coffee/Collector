
#ifndef filter__field_mapping_h
#define filter__field_mapping_h

#include "cache/proc_cache_struct.hpp"
#include "rule/loader.hpp"
#include "schema/schema.hpp"


namespace filter {

enum : int64_t
{
    OID__UNKNOWN        = -1,   // 未知结构
    OID__PROCESS        = 101,  // 进程对象
    OID__PROCESS_FILE   = 101,  // 进程可执行文件对象
    OID__PPROCESS       = 120,  // 父进程对象
    OID__PPROCESS_FILE  = 121,  // 父进程可执行文件对象
    OID__FILE           = 200,  // 文件对象
    OID__NEW_FILE       = 201,  // 文件重命名的新文件对象
    OID__NET_DNS        = 300,  // DNS请求对象
    OID__NET_CONNECT    = 301,  // 网络连接对象
};

enum : int64_t
{
    FID_UNKNOWN             = -1,       // 未知字段

    FID_PROC__PID           = 1001,     // PID
    FID_PROC__GID           = 1002,     // 进程组ID
    FID_PROC__PPID          = 1003,     // 父进程PID
    FID_PROC__NAME          = 1004,     // 进程名称
    FID_PROC__PATH          = 1005,     // 可执行文件路径
    FID_PROC__ARGS          = 1006,     // 进程命令行参数
    FID_PROC__WORK_DIR      = 1007,     // 工作目录
    FID_PROC__UID           = 1008,     // 进程UID
    FID_PROC__SID           = 1009,     // 进程会话ID

    FID_FILE__NAME          = 2001,     // 文件名称
    FID_FILE__PATH          = 2002,     // 文件路径

    FID_NET__PROTOCOL       = 3001,     // 网络协议
    FID_NET__IS_IPV6        = 3002,     // IPv6标记
    FID_NET__IS_INBOUND     = 3003,     // 出向标记
    FID_NET__SOURCE_PORT    = 3004,     // 源端口
    FID_NET__SOURCE_IP      = 3005,     // 源地址
    FID_NET__DEST_PORT      = 3006,     // 目标端口
    FID_NET__DEST_IP        = 3007,     // 目标地址

    FID_DNS__DOMAIN         = 4001,     // 请求域名
    FID_DNS__QUERY_STATUS   = 4002,     // 请求结果
    FID_DNS__ANSWER         = 4003,     // 应答域名               (不支持过滤)
    FID_DNS__ANSWER_TYPE1   = 4003,     // 应答域名，type-1类型    (不支持过滤)
    FID_DNS__ANSWER_TYPE5   = 4005,     // 应答域名，type-1类型    (不支持过滤)
};

typedef struct StructSet
{
public:
    StructSet();
public:
    ProcInfoPtr proc;
    ProcInfoPtr pproc;
    SCHEMA      *msg;
} StructSet;

typedef struct FeildKey
{
public:
    int64_t oid{-1};
    int64_t fid{-1};
public:
    bool operator <(const FeildKey &i_other) const
    {
        if(this->oid != i_other.oid) {
            return (this->oid < i_other.oid);
        }
        return (this->fid < i_other.fid);
    }
} FeildKey;


typedef std::function< std::string(const StructSet &i_structs) > FeildExtractor;

const FeildExtractor& new_extractor(const rule::ConditionPtr &i_rule);


}/* namespace aegis_mix */


#endif /* filter__field_mapping_h */
