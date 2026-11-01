
#ifndef bpf_link_arg_h
#define bpf_link_arg_h

#include <list>
#include <string>

namespace bpf { // namespace bpf

typedef struct LinkArg
{
public:
    std::string         hook_type;
    std::string         hook_name;
    std::string         func_name;
public:
    void clear() {
        hook_type.clear();
        hook_name.clear();
        func_name.clear();
    }
} LinkArg;
typedef std::list<LinkArg> LinkArgs;

} // namespace bpf

#endif /* bpf_link_arg_h */
