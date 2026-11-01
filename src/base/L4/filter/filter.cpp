
#include "filter/filter.hpp"

namespace filter {

ret_code Filter::load(const rule::Loader &i_loader)
{
    ret_code r_code = RC_OK;
        
    if(i_loader.enable_default_on_match()) {
        _filter.enable_default_on_match(true);
    }
    r_code = _filter.load(i_loader);

    return r_code;
}

ret_code Filter::match(uint64_t i_event, const StructSet &i_structs)
{
    std::string null; return this->match(i_event, i_structs, null);
}

ret_code Filter::match(uint64_t i_event, const StructSet &i_structs, std::string &o_id)
{
    return _filter.match(i_event, i_structs, o_id);
}

}
