
#include "filter/filter__sub_filter.hpp"

namespace filter { /* namespace aegis_mix */

ret_code SubFilter::load(const rule::Loader &i_loader)
{
    ret_code r_code = RC_OK;

    do {
        for(const auto &[event, group] : i_loader.get_filter_rule_groups()) {
            auto &subfilter_group = _filters[event];
            
            uint64_t include_rule_counter = 0;
            
            for(const auto &rule : group->rules) {
                subfilter_group.resolvers.emplace_back( Resolver::create(rule) );
                
                if(ACTION_INCLUDE == rule->action) {
                    include_rule_counter += 1;
                }
            }
            
            subfilter_group.rule_group       = group;
            subfilter_group.on_match_default = ((include_rule_counter == 0) ? ACTION_INCLUDE : ACTION_EXCLUDE);
        }
    } while (false);

    return r_code;
}

ret_code SubFilter::match(uint64_t i_event_type, const StructSet &i_structs)
{
    std::string null; return this->match(i_event_type, i_structs, null);
}

ret_code SubFilter::match(uint64_t i_event_type, const StructSet &i_structs, std::string &o_id)
{
    ret_code r_code = RC_FILTER_MATCH_UNKNOWN;
    
    do {
        const auto &filter = _filters.find(i_event_type);
        if(_filters.end() == filter) {
            break;
        }
        
        const ResolverGroup &group = filter->second;
        
        int64_t gid = -1;
        int64_t rid = -1;
        int64_t cid = -1;
        
        Action on_match = enable_default_on_match() ? group.on_match_default : ACTION_UNKNOWN;
        
        for(const auto &resolver : group.resolvers) {
            if(resolver->match(i_structs, cid)) {
                rid      = resolver->get_rule()->id;
                on_match = static_cast<Action>(resolver->get_rule()->action);
                
                break;
            } cid = -1;
        }
        gid = group.rule_group->id;
        
        do {
            if(gid >= 0) { o_id.assign(std::to_string(gid));        } else { break; }
            if(rid >= 0) { o_id.append("-" + std::to_string(rid));  } else { break; }
            if(cid >= 0) { o_id.append("-" + std::to_string(cid));  } else { break; }
        } while (false);
        
        switch (on_match) {
            case ACTION_INCLUDE: { r_code = RC_FILTER_MATCH_INCLUEDE; } break;
            case ACTION_EXCLUDE: { r_code = RC_FILTER_MATCH_EXCLUEDE; } break;
            default: {} break;
        }
    } while (false);
    
    return r_code;
}

} /* namespace aegis_mix */
