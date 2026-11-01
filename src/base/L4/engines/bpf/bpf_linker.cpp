
#include <set>

#include <unistd.h>

#include "libbpf.h"
#include "engines/bpf/bpf_link_type.hpp"
#include "engines/bpf/bpf_prog_info.hpp"

#include "engines/bpf/bpf_linker.hpp"

namespace bpf { // namespace bpf

LinkGroup::LinkGroup(struct bpf_object *i_bpf_obj)
{
    _obj = i_bpf_obj;
}
LinkGroup::~LinkGroup()
{
    this->unlink();
}

ret_code LinkGroup::link(const LinkArgs &i_arg)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(i_arg.empty())   { r_code = RC_ARG_ERR;             break; }
        if(nullptr == _obj) { r_code = RC_BPF_LINKER_OBJ_NULL; break; }
        if(!_links.empty()) { r_code = RC_BPF_LINKER_LINK_ERR; break; }

        for(const auto &arg : i_arg) {
            struct bpf_link     *link = nullptr;
            struct bpf_program  *prog = nullptr;

            prog = bpf_object__find_program_by_name(_obj, arg.func_name.c_str());
            if(nullptr == prog) {
                r_code = RC_BPF_PROG_FIND_ERR; continue;
            }

            link = this->attach(prog, arg);

            if(0 == libbpf_get_error(link)) {
                r_code = RC_OK; _links.push_back(link);
            } else {
                r_code = RC_BPF_PROG_ATTACH_ERR; break;
            }
        }

        // 一个LinkGroup中，任意一个Link附加失败，均算作整个LinkGroup附加失败，所以需要unlink()
        if(RC_OK != r_code) { this->unlink(); }
    } while (false);

    return r_code;
}

ret_code LinkGroup::unlink()
{
    for(auto link : _links) {
        bpf_link__destroy(link);
    } _links.clear();

    return RC_OK;
}

struct bpf_link * LinkGroup::attach(struct bpf_program *i_prog, const LinkArg &i_arg)
{
    static const std::set<std::string> kprobe     = { "kprobe"   };
    static const std::set<std::string> tracepoint = { "syscalls" };

    const auto &type = i_arg.hook_type;
    if(kprobe.end() != kprobe.find(type)) {
        return bpf_program__attach_kprobe(i_prog, i_arg.hook_type.empty(), i_arg.hook_name.c_str());
    }
    if(tracepoint.end() != tracepoint.find(type)) {
        return bpf_program__attach_tracepoint(i_prog, i_arg.hook_type.c_str(), i_arg.hook_name.c_str());
    }

    return nullptr;
}

} // namespace bpf
