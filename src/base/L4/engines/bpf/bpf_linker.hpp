
#ifndef bpf_linker_h
#define bpf_linker_h

#include "engines/bpf/bpf_link_arg.hpp"
#include "engines/event_observer_manager.hpp"

namespace bpf { // namespace bpf

class LinkGroup : public EventObserverManager
{
public:
     explicit LinkGroup(struct bpf_object *);
             ~LinkGroup();
public:
    ret_code   link(const LinkArgs &);
    ret_code unlink();
private:
    struct bpf_link *attach(struct bpf_program *i_prog, const LinkArg &i_arg);
private:
    struct bpf_object           *_obj;
    std::list<struct bpf_link *> _links;
};
typedef std::shared_ptr<LinkGroup>  LinkGroupPtr;
#define NEW_BPF_LINK_GROUP(i_args)  std::make_shared<bpf::LinkGroup>(i_args)

} // namespace bpf


#endif /* bpf_linker_h */
