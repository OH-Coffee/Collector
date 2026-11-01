
#ifndef consts_hpp
#define consts_hpp

#pragma mark --- Socket
#define SOCK_PATH_DATA                  "/tmp/cardinal.sock"

#define NAME_NET_EXTENSION              "NET"
#define NAME_DNS_EXTENSION              "DNS"


#pragma mark --- Sleep时间
#define SLEEP_TIME_DEEP_MS              (10 * 1000)
#define SLEEP_TIME_LIGHT_MS             ( 5 * 1000)


#if defined(OS_MACOS)
    #define HOME_DIR                    "/Library/Application Support/Threatbook/Agent"
#elif defined(OS_LINUX)
    #define HOME_DIR                    "/opt/Threatbook/Agent"
#endif
#define HOME_DIR_DATA                   HOME_DIR"/data"
#define HOME_DIR_CARDINAL               HOME_DIR"/cardinal"
#define HOME_DIR_SECURITY               HOME_DIR"/security"

#pragma mark --- PID文件
#define PID_PATH_CARDINAL               HOME_DIR_DATA"/proc/tbCardinal.pid"
#define PID_PATH_NET_EXTENSION          HOME_DIR_DATA"/proc/" ID_NET_EXTENSION ".pid"
#define PID_PATH_DNS_EXTENSION          HOME_DIR_DATA"/proc/" ID_DNS_EXTENSION ".pid"

#pragma mark --- 配置路径
#define LOCK_PATH_CARDINAL              HOME_DIR_DATA"/lock/tbCardinal.lock"

#define CONF_PATH_CARDINAL              HOME_DIR_CARDINAL"/cardinal.conf"
#define CONF_PATH_NET_EXTENSION         HOME_DIR_CARDINAL"/net-extension.conf"
#define CONF_PATH_DNS_EXTENSION         HOME_DIR_CARDINAL"/dns-extension.conf"

#define RULE_PATH_INTERNAL_WHITELIST    HOME_DIR_SECURITY"/Aegis/AegisDLH.dat"


#pragma mark --- 日志配置
#define LOG_MAX_FILE_SIZE               (5 * 1024 * 1024)       // 单位：字节
#define LOG_MAX_BUFFER_SIZE             (5 * 1024)              // 单位：字节

#define LOG_DIR                         "/var/log/tbagent/Cardinal"
#define LOG_PATH_CARDINAL               (LOG_DIR"/cardinal.log")
#define LOG_PATH_COLLECTOR_CMD           (LOG_DIR"/cardinal-cmdline.log")
#define LOG_PATH_NET_EXTENSION          (LOG_DIR"/net-extension.log")
#define LOG_PATH_DNS_EXTENSION          (LOG_DIR"/dns-extension.log")


#pragma mark --- 资源配置
#define LIMIT_MAX_CORE                  500     // 单位：MB
#define LIMIT_MAX_RSS                   500     // 单位：MB
#define LIMIT_MAX_NOFILE                1024


#pragma mark --- 其他
#define SWITCH_ON                       1
#define SWITCH_OFF                      0

#define BPF_PROG_DIR                    HOME_DIR_CARDINAL"/programs"

#define IS_SET(i_set, i_flag) ((i_flag) == ((i_set) & (i_flag)))


#endif /* consts_hpp */

