
#ifndef filter__aegis_mix_h
#define filter__aegis_mix_h

#include "filter/filter__sub_filter.hpp"

namespace filter {

class Filter
{
public:
     Filter() = default;
    ~Filter() = default;
public:
    ret_code load(const rule::Loader &i_loaders);
public:
    ret_code match(uint64_t i_event, const StructSet &i_structs);
    ret_code match(uint64_t i_event, const StructSet &i_structs, std::string &o_id);
private:
    SubFilter _filter;
};

/* ---------------------------------------------------------------------------------------------------- */

}/* namespace aegis_mix */

#endif /* filter__aegis_mix_h */
