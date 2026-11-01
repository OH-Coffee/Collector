
#ifndef string_split_hpp
#define string_split_hpp

#include <set>
#include <list>
#include <vector>
#include <functional>
#include <string>
#include <string_view>

#include "string/string_detail.hpp"

template <typename T>
T string_split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty_str = false)
{
    T r_data;

    do {
        if(i_str.empty() || i_delim.empty()) {
            string_add2(r_data, i_str, i_drop_empty_str); break;
        }

        size_t s_site = 0, e_site = 0;
        while ( std::string::npos != ( e_site = i_str.find(i_delim, s_site) ) ) {
            string_add2(r_data, i_str.substr(s_site, e_site - s_site), i_drop_empty_str);
            s_site = e_site + i_delim.size();
        }

        string_add2(r_data, i_str.substr(s_site), i_drop_empty_str);
    } while (false);

    return r_data;
}

template <typename T>
T string_rsplit(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty_str = false)
{
    T r_data;

    do {
        if(i_str.empty() || i_delim.empty()) {
            string_add2(r_data, i_str, i_drop_empty_str); break;
        }
        
              size_t str_size   = i_str.size();
        const size_t delim_size = i_delim.size();
        
        while(str_size > 0) {
            const size_t tail_pos  = (str_size > delim_size) ? (str_size - delim_size) : 0;
            const size_t delim_pos = i_str.rfind(i_delim, tail_pos);

            if(delim_pos != std::string_view::npos) {
                const size_t head_pos = delim_pos + delim_size;
                
                string_add2(r_data, i_str.substr(head_pos, str_size - head_pos), i_drop_empty_str);

                str_size = delim_pos;
            } else {
                string_add2(r_data, i_str.substr(0, str_size), i_drop_empty_str); break;
            }
        }
    } while (false);

    return r_data;
}


extern template std::set<std::string>    string_split(const std::string_view &, const std::string_view &, bool);
extern template std::list<std::string>   string_split(const std::string_view &, const std::string_view &, bool);
extern template std::vector<std::string> string_split(const std::string_view &, const std::string_view &, bool);

extern template std::set<std::string>    string_rsplit(const std::string_view &, const std::string_view &, bool);
extern template std::list<std::string>   string_rsplit(const std::string_view &, const std::string_view &, bool);
extern template std::vector<std::string> string_rsplit(const std::string_view &, const std::string_view &, bool);


void string_split(const std::string_view &i_str,
                  const std::string_view &i_delim,
                  const std::function<bool(const std::string_view &)> &i_callback);

void string_rsplit(const std::string_view &i_str,
                   const std::string_view &i_delim,
                   const std::function<bool(const std::string_view &)> &i_callback);


#endif /* string_split_hpp */
