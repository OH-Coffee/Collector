
#ifndef cardinal_event_body_hpp
#define cardinal_event_body_hpp

#ifdef OS_MACOS
    #import <memory>
    #import <bsm/libbsm.h>
    #import <Foundation/Foundation.h>
    #import <EndpointSecurity/EndpointSecurity.h>
#endif

#define BUF_LEN_CLOSE_PATH      256
#define BUF_LEN_EXEC_PATH       256
#define BUF_LEN_EXEC_ARGV       512
#define BUF_LEN_RECV_DATA       1024
#define BUF_LEN_RENAME_PATH     512

#pragma pack(push, 8)

#ifdef OS_MACOS
    typedef struct EventBodyEndpointSecurity
    {
    public:
         EventBodyEndpointSecurity(es_client_t *i_client, const es_message_t *i_message) {
             this->client  = i_client;
             this->message = es_copy_message(i_message);
         }
        ~EventBodyEndpointSecurity() {
            if(nil != this->message) { es_free_message(this->message); }
        }
    public:
        es_client_t  *client  {nil};
        es_message_t *message {nil};
    } EventBodyEndpointSecurity;

    typedef std::shared_ptr<EventBodyEndpointSecurity> EventBodyEndpointSecurityPtr;
    #define NEW_EVENT_BODY_ENDPOINT_SECURITY(i_args...) std::make_shared<EventBodyEndpointSecurity>(i_args)
#endif

typedef struct
{
    int pid_parent;
    int pid_child;
} EventBodyClone;

typedef struct
{
    int64_t                 fd;
    int64_t                 addr_len;
    struct sockaddr_storage addr;
} EventBodyConnect;

typedef struct
{
    char        path[BUF_LEN_EXEC_PATH];
    char        argv[BUF_LEN_EXEC_ARGV];
} EventBodyExec;

typedef struct
{
    bool    _exit;
    int64_t exit_code;
    int64_t exit_signal;
} EventBodyExitGroup;

typedef struct
{
    char path[BUF_LEN_CLOSE_PATH];
} EventBodyWClose;

typedef struct
{
    char path[BUF_LEN_CLOSE_PATH];
} EventBodyRClose;

typedef struct
{
    int64_t         fd;
    struct sockaddr addr;
    int64_t         addr_len;
    char            data[BUF_LEN_RECV_DATA];
    int64_t         data_len;
} EventBodyRecvfrom;

typedef struct
{
    int64_t         fd;
    struct sockaddr addr;
    int64_t         addr_len;
    char            data[BUF_LEN_RECV_DATA];
    int64_t         data_len;
} EventBodyRecvmsg;

typedef struct
{
    int             reserve1;
    int             reserve2;
    char            reserve3[32]; // 先预留40字节
    int64_t         data_len;
    // data必须放最后
    char            data[BUF_LEN_RECV_DATA];
} EventBodySkbConsumeUdp;

typedef struct
{
    int64_t         olddfd;
    int64_t         newdfd;
    char            oldname[BUF_LEN_RENAME_PATH];
    char            newname[BUF_LEN_RENAME_PATH];
    uint64_t        flags;
} EventBodyRename, EventBodyRenameat, EventBodyRenameat2;

typedef struct
{
    int64_t         msg_len;
    char            msg[1];             // 这里存放的是SCHEMA结构体，实际事件内容，变长结构
} EventBodyUDDS;

typedef struct
{
    struct {
        uint64_t    inode;
        uint64_t    dev_maj;
        uint64_t    dev_min;
    } mount;
    char            oldname[BUF_LEN_RENAME_PATH];
    char            newname[BUF_LEN_RENAME_PATH];
} EventBodyVfsRename;

#pragma pack(pop)

#endif // cardinal_event_body_hpp
