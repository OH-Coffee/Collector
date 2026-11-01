#ifndef schema__schema_body_hpp
#define schema__schema_body_hpp

#include <cstdint>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#define MSG_NAME_SIZE     128
#define MSG_STR_IP_SIZE   MSG_NAME_SIZE
#define MSG_STR_ID_SIZE   MSG_NAME_SIZE
#define MSG_PATH_SIZE     512
#define MSG_TEXT_SIZE     MSG_PATH_SIZE
#define MSG_STR_BUF_SIZE  1024

typedef struct
{
    uint32_t uid;
    uint32_t gid;
    
    struct timespec atime;
    struct timespec mtime;
    struct timespec ctime;

    size_t          st_size;
    mode_t          st_mode;
    nlink_t         st_nlink;
} FileStat;

typedef struct PROC_GEN_INFO {
    pid_t           pid;
    pid_t           ppid;

    uid_t           uid;
    pid_t           gid;
    pid_t           sid;
    
    struct timeval  start_time;

    uint8_t         name[MSG_NAME_SIZE];
    uint8_t         path[MSG_PATH_SIZE];
    uint8_t         args[MSG_STR_BUF_SIZE];
    uint8_t         work_dir[MSG_PATH_SIZE];
    FileStat        path_stat;

    uint8_t         tty[MSG_PATH_SIZE];
    uint8_t         env[MSG_STR_BUF_SIZE];

    uint8_t         ppath[MSG_PATH_SIZE];
} EVENT_PROC_EXEC, EVENT_PROC_FORK, EVENT_PROC_EXIT;

typedef struct FILE_GEN_INFO
{
    FileStat        stat;
    int32_t         flags;
    uint8_t         path[MSG_PATH_SIZE];
    uint8_t         target_path[MSG_PATH_SIZE];
    
    uint8_t         path_proc[MSG_PATH_SIZE];
    uint8_t         path_pproc[MSG_PATH_SIZE];
} EVENT_FILE_OPEN, EVENT_FILE_RENAME, EVENT_FILE_RCLOSE, EVENT_FILE_WCLOSE;

typedef struct
{
    uint64_t        protocol;
    uint16_t        src_port;
    uint16_t        dst_port;
    uint8_t         src_addr[MSG_STR_IP_SIZE];
    uint8_t         dst_addr[MSG_STR_IP_SIZE];

    uint8_t         is_ipv6;
    uint8_t         is_inbound;
    
    uint8_t         path_proc[MSG_PATH_SIZE];
    uint8_t         path_pproc[MSG_PATH_SIZE];
} EVENT_NET_CONNECT;

typedef struct
{
    uint64_t    query_type;
    uint64_t    query_options;
    uint64_t    query_result;
    uint8_t     query_name[MSG_TEXT_SIZE];
    uint8_t     answers[MSG_TEXT_SIZE];
    
    uint8_t         path_proc[MSG_PATH_SIZE];
    uint8_t         path_pproc[MSG_PATH_SIZE];
} EVENT_NET_DNS;

typedef union
{
    EVENT_PROC_EXEC         exec;
    EVENT_PROC_FORK         fork;
    EVENT_PROC_EXIT         exit;
    EVENT_FILE_OPEN         open;
    EVENT_FILE_RENAME       rename;
    EVENT_FILE_RCLOSE       rclose;
    EVENT_FILE_WCLOSE       wclose;
    EVENT_NET_DNS           dns;
    EVENT_NET_CONNECT       connect;
    char                    str[0];
} SCHEMA_BODY;

#endif // schema__schema_body_hpp