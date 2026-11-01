
#ifndef bpf_loader_args_hpp
#define bpf_loader_args_hpp

#include <list>

#include "engines/event_observer.hpp"

#define BPF_BUFF_DEFAULT_MEM_SIZE   (1 * 1024 * 1024)                       // 1MB
#define BPF_BUFF_MAX_MEM_SIZE       (32 * BPF_BUFF_DEFAULT_MEM_SIZE)        // 32MB
#define BPF_BUFF_DEFAULT_PAGE_NUM   (BPF_BUFF_DEFAULT_MEM_SIZE / 4096)      // 256

namespace bpf { // namespace bpf

typedef struct ProgInfo
{
public:
    std::string     path;
    std::string     buff_name;
    int64_t         buff_size = BPF_BUFF_DEFAULT_MEM_SIZE;
public:
    void clear() {
        path        = "";
        buff_name   = "";
        buff_size   = 0;
    }
} ProgInfo;

} // namespace bpf




#endif // bpf_loader_args_hpp
