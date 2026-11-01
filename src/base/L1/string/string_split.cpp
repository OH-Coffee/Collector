
#include "string/string_split.hpp"


template std::set<std::string>    string_split(const std::string_view &, const std::string_view &, bool);
template std::list<std::string>   string_split(const std::string_view &, const std::string_view &, bool);
template std::vector<std::string> string_split(const std::string_view &, const std::string_view &, bool);

template std::set<std::string>    string_rsplit(const std::string_view &, const std::string_view &, bool);
template std::list<std::string>   string_rsplit(const std::string_view &, const std::string_view &, bool);
template std::vector<std::string> string_rsplit(const std::string_view &, const std::string_view &, bool);

void string_split(const std::string_view &i_str,
                  const std::string_view &i_delim,
                  const std::function<bool(const std::string_view &)> &i_callback)
{
    if(i_str.empty() || i_delim.empty()) { i_callback(i_str); return; }

    size_t s_site = 0, e_site = 0;
    
    bool go_on = true;
    while ( std::string::npos != ( e_site = i_str.find(i_delim, s_site) ) ) {
        go_on = i_callback( i_str.substr(s_site, e_site - s_site) );
        if(!go_on) {
            break;
        }
        s_site = e_site + i_delim.size();
    }
    
    if(go_on && s_site < i_str.size()) { i_callback( i_str.substr(s_site) ); }
}

void string_rsplit(const std::string_view &i_str,
                   const std::string_view &i_delim,
                   const std::function<bool(const std::string_view &)> &i_callback)
{
    if(i_str.empty() || i_delim.empty()) { i_callback(i_str); return; }
    
          size_t str_size   = i_str.size();
    const size_t delim_size = i_delim.size();
    
    while(str_size > 0) {
        const size_t tail_pos  = (str_size > delim_size) ? (str_size - delim_size) : 0;
        const size_t delim_pos = i_str.rfind(i_delim, tail_pos);

        if(delim_pos != std::string_view::npos) {
            const size_t head_pos = delim_pos + delim_size;
            
            if(!i_callback( i_str.substr(head_pos, str_size - head_pos) )) {
                break;
            }
            str_size = delim_pos;
        } else {
            i_callback( i_str.substr(0, str_size) ); break;
        }
    }
}
