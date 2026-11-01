
#include <atomic>
#include <filesystem>

#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "logger.hpp"

#define PRINT_STRING_LINE   ">>\n"

int g_max_log_file      = 5;
int g_max_log_file_size = 5 * 1024 * 1024;
int g_max_log_level     = LOG_LEVEL_TRACE;
int g_max_buffer_size   = 10 * 1024;

static const char * const g_log_level_str[] = {
    " DIAG",
    "DEBUG",
    "TRACE",
    " INFO",
    " WARN",
    "ERROR",
    " CRIT",
};

static FILE* g_file     = nullptr;
static char* g_filename = nullptr;
static char* g_filepath = nullptr;

static pthread_mutex_t log_lock;
static pthread_mutex_t buf_lock;

static int                  g_screen = 0;
static std::atomic_uint64_t g_total  = 0;

#define MAX_BUF_LINE        32
static char* g_buf[MAX_BUF_LINE];
static int g_buf_cnt = 8;
static int g_buf_cur = 0;

int g_back_up_log   = 1;

static int64_t get_file_size(const char* filename)
{
    struct stat fs;
    if (stat(filename, &fs) == -1) {
        return -1;
    }
    return fs.st_size;
}

static bool make_log_dir(const std::string &i_log_file_path)
{
    bool ret_bool = false;
    
    do {
        std::string log_dir = i_log_file_path.substr(0, i_log_file_path.rfind('/'));
        if(log_dir.size() <= 0 || log_dir[0] != '/')   { ret_bool = false; break; }
        
        if(0 == access(log_dir.c_str(), F_OK)) { ret_bool = true; break; }
        
        std::error_code ec;
        ret_bool = std::filesystem::create_directories(log_dir, ec);
    } while (0);
    
    return ret_bool;
}

int log_init(const std::string &i_filepath, int i_log_level, int i_screen)
{
    if(!make_log_dir(i_filepath))   { return -1; }
    
    g_file = fopen(i_filepath.c_str(), "a+");
    if (g_file == nullptr) {
        fprintf(stderr, "open file %s failed error: %s\n", i_filepath.c_str(), strerror(errno));
        return -1;
    }
    
    g_filepath = strdup(i_filepath.c_str());
    g_filename = strdup(i_filepath.substr(i_filepath.rfind("/") + 1).c_str());
    
    if (get_file_size(i_filepath.c_str()) > g_max_log_file_size) {
        log_backup(i_filepath.c_str(), nullptr, g_back_up_log);
    }
    
    g_max_log_level = ( i_log_level >= LOG_LEVEL_DIAGN ? i_log_level : LOG_LEVEL_TRACE);
    
//    if (i_buf_line >= MAX_BUF_LINE) {
//        g_buf_cnt = MAX_BUF_LINE;
//    } else {
//        g_buf_cnt = i_buf_line;
//    }
    
    for (int i = 0; i < g_buf_cnt; i++) {
        // not free, system free
        g_buf[i] = reinterpret_cast<char *>(malloc(static_cast<size_t>(g_max_buffer_size)));
        if (g_buf[i] == nullptr) {
            return -3;
        } else {
            bzero(g_buf[i], g_max_buffer_size);
        }
    }
    
    if (0 != pthread_mutex_init(&log_lock, nullptr)) {
        fprintf(stderr, "pthread_mutex_init fail: %s\n", strerror(errno));
        return -2;
    }
    if (0 != pthread_mutex_init(&buf_lock, nullptr)) {
        fprintf(stderr, "pthread_mutex_init fail: %s\n", strerror(errno));
        return -2;
    }
    
    g_screen = i_screen;
    return 0;
}

void log_flush()
{
     if (g_file != nullptr) { fflush(g_file); }
}

void log_uninit()
{
    for (int i = 0; i < g_buf_cnt; i++) { free(g_buf[i]); }
    
    pthread_mutex_destroy(&log_lock);
    pthread_mutex_destroy(&buf_lock);
    
    if(nullptr != g_file    ) { fclose(g_file);   g_file     = nullptr; }
    if(nullptr != g_filename) { free(g_filename); g_filename = nullptr; }
    if(nullptr != g_filepath) { free(g_filepath); g_filepath = nullptr; }
}

static int file_filter(const struct dirent *i_dir)
{
    if(0 == strncmp(g_filename, i_dir->d_name, strlen(g_filename))) {
        return ( (i_dir->d_type == DT_REG) ? 1 : 0 );
    }
    return 0;
}

static int clean_log_file(const std::string &i_dir)
{
    int             file_list_len = 0;
    struct dirent **file_list_ptr = nullptr;
    
    do {
        if(i_dir    == "/") { break; }
        if(i_dir[0] != '/') { break; }
        
        file_list_len = scandir(i_dir.c_str(), &file_list_ptr, file_filter, alphasort);
        for(int i = 0; i < (file_list_len - g_max_log_file); ++i) {
            remove(std::string(i_dir + "/" + file_list_ptr[i]->d_name).c_str());
        }
    } while (0);
    
    if(file_list_ptr != nullptr) {
        for(int i = 0; i < file_list_len; ++i) { free(file_list_ptr[i]); }
        free(file_list_ptr);
    }
    
    return file_list_len;
}

FILE* log_backup(const std::string &i_filepath, FILE* i_fp, int is_backup)
{
    if(0 != access(i_filepath.c_str(), F_OK)) { return nullptr; }
    
    if (is_backup) {
        char new_path[128];
        snprintf(new_path, sizeof(new_path), "%s.%u", i_filepath.c_str(), (uint32_t)time(nullptr));
        
        unlink(new_path); rename(i_filepath.c_str(), new_path);
    } else {
        unlink(i_filepath.c_str());
    }
    
    clean_log_file(i_filepath.substr(0, i_filepath.rfind('/')));

    if (i_fp) {
        fclose(i_fp); remove(i_filepath.c_str());
        return fopen(i_filepath.c_str(), "a+");
    } else {
        return nullptr;
    }
}

void log_out(int i_level, const char *i_file, const int i_line, const char *i_func, const char *i_format, ...)
{
    if(i_file) { ; }
    if(i_line) { ; }
    if(i_func) { ; }

    if (i_level < g_max_log_level)  { return; }
    
    va_list va;
    
    char* message = nullptr;
    
    pthread_mutex_lock(&buf_lock);
    int idx;
    for (int i = 0; i<= g_buf_cnt; i++) {
        if (i == g_buf_cnt) {
            usleep(1000 * 10);
            i = 0;
            continue;
        }
        
        idx = (i + g_buf_cur) % g_buf_cnt;
        
        if (g_buf[idx][0] == '\0') {
            message = g_buf[idx];
            g_buf_cur = idx + 1;
            message[0] = '\1';
            break;
        }
    }
    pthread_mutex_unlock(&buf_lock);
    
    if (message == nullptr) { return; }
    
    va_start(va, i_format);
    
    time_t now = time(nullptr);
    char timebuf[50];
    strftime(timebuf, sizeof(timebuf), "%F %H:%M:%S", localtime(&now));

    static pid_t pid = getpid();
    
    int size = 0;

    size += snprintf(message, (size_t)(g_max_buffer_size), "%s [%s] [%d] >>> ", timebuf, g_log_level_str[i_level], pid);
    size += vsnprintf(message + size, (size_t)(g_max_buffer_size - size), i_format, va);
    
#ifdef LOG_DETAIL
    size += snprintf(message + size,  (size_t)(g_buffer_size - size), "\t(%s:%d:%s)", i_file, i_line, i_func);
#endif
    
    if (size > g_max_buffer_size) {
        size = g_max_buffer_size - 1;
        snprintf(message + g_max_buffer_size - sizeof(PRINT_STRING_LINE), sizeof(PRINT_STRING_LINE), PRINT_STRING_LINE);
    } else {
        message[size++] = '\n';
    }
    
    pthread_mutex_lock(&log_lock);
        if(nullptr == g_file) {
            g_file = fopen(g_filepath, "a+");
        } else {
            fwrite(message, 1, static_cast<size_t>(size), g_file); fflush(g_file);
        }

        g_total += static_cast<uint32_t>(size);
    
        if (g_total > (uint64_t)g_max_log_file_size) {
            g_file = log_backup(g_filepath, g_file, g_back_up_log);
            g_total = 0;
        }
        if (g_screen) { fwrite(message, 1, static_cast<size_t>(size), stderr); }
    pthread_mutex_unlock(&log_lock);
    
    va_end(va);
    message[0] = '\0';
}

int log_level()
{
    return g_max_log_level;
}

int log_file_size(int i_size)
{
    return ( g_max_log_file_size = ( i_size > 0 ? i_size : g_max_log_file_size) );
}

int log_buffer_size(int i_size)
{
    return ( g_max_buffer_size = ( i_size > 0 ? i_size : g_max_buffer_size) );
}

