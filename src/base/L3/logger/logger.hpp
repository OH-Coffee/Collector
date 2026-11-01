
#ifndef logger_hpp
#define logger_hpp

#include <string>

#define LOG_LEVEL_DIAGN     0
#define LOG_LEVEL_DEBUG     1
#define LOG_LEVEL_TRACE     2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_WARN      4
#define LOG_LEVEL_ERROR     5
#define LOG_LEVEL_CRIT      6

#ifdef LOG_DETAIL
    #define GET_F   __FILE__
    #define GET_L   __LINE__
    #define GET_M   __FUNCTION__
#else
    #define GET_F   ""
    #define GET_L   0
    #define GET_M   ""
#endif // LOG_DETAIL

#ifdef __cplusplus
extern "C" {
#endif
    int     log_init(const std::string &i_file_path, int i_log_level = LOG_LEVEL_INFO, int i_screen = 0);
    void    log_flush();
    FILE*   log_backup(const std::string &i_file_path, FILE* i_fp, int is_backup);
    void    log_uninit();

    void    log_out(int i_level, const char *i_file, const int i_line, const char *i_func, const char *i_format, ...);

    int     log_level();
    int     log_file_size(int i_size = 0);
    int     log_buffer_size(int i_size = 0);

    #define LOG_DIAGN(format, ...)  log_out(LOG_LEVEL_DIAGN, GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_DEBUG(format, ...)  log_out(LOG_LEVEL_DEBUG, GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_TRACE(format, ...)  log_out(LOG_LEVEL_TRACE, GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_INFO(format, ...)   log_out(LOG_LEVEL_INFO,  GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_WARN(format, ...)   log_out(LOG_LEVEL_WARN,  GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_ERROR(format, ...)  log_out(LOG_LEVEL_ERROR, GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
    #define LOG_CRIT(format, ...)   log_out(LOG_LEVEL_CRIT,  GET_F, GET_L, GET_M, format, ##__VA_ARGS__)
#ifdef __cplusplus
}
#endif

#endif  /* logger_hpp */

