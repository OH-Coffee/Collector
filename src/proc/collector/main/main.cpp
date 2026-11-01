
#include "collector.hpp"

#include "file/file_api.hpp"
#include "string/string_api.hpp"
#include "process/proc_info.hpp"
#include "logger/logger.hpp"

#include "config/collector_config.hpp"

int main(int argc, const char * argv[])
{
    // if(getuid() != 0) {
    //     printf("Please run with root privileges!\n"); exit(-1);
    // }
    
    umask(022);

    std::string dir = file_dir(proc_path(getpid()));
    
    CollectorConfig::ins().load(dir + "../config/collector.json");
    
#if defined(OS_MACOS)
    @autoreleasepool {
        std::thread worker([argc, argv](){
#endif
            if(0 != strcmp(argv[1], "Arch-V")) {
                ret_code r_code = RC_UNKNOWN;
                
                int         log_level = CollectorConfig::ins().get_int32("log.cmd_log_level");
                std::string log_path  = CollectorConfig::ins().get_string("log.cmd_log_path");

                log_init(log_path.c_str(), log_level);
                {
                    // r_code = handler_for_cmdline(argc, argv);
                }
                log_uninit(); exit(r_code);
            } else {
                int         log_level = CollectorConfig::ins().get_int32("log.level");
                std::string log_path  = CollectorConfig::ins().get_string("log.path");

                log_file_size(  CollectorConfig::ins().get_int32("log.max_file_size"));
                log_buffer_size(CollectorConfig::ins().get_int32("log.max_buffer_size"));
                log_init(log_path.c_str(), log_level);

                LOG_INFO("[Collector] 开始启动，PID: %d", getpid());
                {
                    do {
                        if(RC_OK != Collector::ins().init())     { break; }
                        if(RC_OK != Collector::ins().start())    { break; }
                    } while (false);
                    
                    Collector::ins().stop();
                    Collector::ins().uninit();
                }
                LOG_INFO("[Collector] 程序退出，PID: %d", getpid());
                
                log_uninit();
            }
            exit(0);
#if defined(OS_MACOS)
        });
        worker.detach();
        
        [[NSRunLoop currentRunLoop] run];
    }
#endif

    return 0;
}

