#ifndef string_transform_hpp
#define string_transform_hpp

#include <set>
#include <string>

#include "string/string_detail.hpp"

std::string string_reverse(const std::string &i_str, const std::string &i_delim);
std::string string_replace(const std::string &i_subject, const std::string &i_target, const std::string &i_replacement);
std::set<std::string> string_replace(const std::string &i_subject, const std::string &i_target, std::set<std::string> i_replacements);
std::set<std::string> string_replace_recursion(const std::string &i_subject, const std::string &i_target, std::set<std::string> i_replacements);

std::string string_to_lower(const std::string &i_str);
std::string string_to_upper(const std::string &i_str);
std::string string_drop_quotes(const std::string &i_str);

std::string string_add_prefix(const std::string &i_str, const std::string &i_prefix);
std::string string_add_suffix(const std::string &i_str, const std::string &i_suffix);
std::string string_surround(const std::string &i_str, const std::string &i_prefix, const std::string &i_suffix);

template <typename T> T string_surround(const T &i_container, const std::string &i_prefix, const std::string &i_suffix)
{
    T r_data;

    for(const auto &it : i_container) {
        string_add2<T>(r_data, string_surround(it, i_prefix, i_suffix), true);
    }

    return r_data;
}

#endif /* string_transform_hpp */
