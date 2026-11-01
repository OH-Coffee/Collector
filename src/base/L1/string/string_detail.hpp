#ifndef string_detail_hpp
#define string_detail_hpp

#include <set>
#include <string>
#include <string_view>
#include <type_traits>

template<typename T> inline void string_add2(T &o_container, const std::string_view &i_str, bool i_drop_empty_str)
{
    if(i_drop_empty_str && i_str.empty()) { return; }

    if constexpr (std::is_same<T, std::set<std::string>>::value) {
        o_container.emplace(i_str);
    } else {
        o_container.emplace_back(i_str);
    }
}

#endif /* string_detail_hpp */
