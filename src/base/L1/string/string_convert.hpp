#ifndef string_convert_hpp
#define string_convert_hpp

#include <cstdlib>
#include <list>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

template <typename T> T string_to_integer(const std::string &i_str)
{
    return (T)(std::is_signed_v<T> ? strtoll(i_str.c_str(), nullptr, 0) : strtoull(i_str.c_str(), nullptr, 0));
}

template <typename T> std::string string_joint(T i_set, char i_delim = ' ')
{
    std::string r_str;
    for(const auto &str : i_set) {
        r_str.append(str); r_str += i_delim;
    }
    if(!r_str.empty()) { r_str.pop_back(); }

    return r_str;
}
extern template std::string string_joint(std::set<std::string>,    char);
extern template std::string string_joint(std::list<std::string>,   char);
extern template std::string string_joint(std::vector<std::string>, char);

std::string string_str2bin(const std::string &i_str);
std::string string_str2bin(const char *i_buf, size_t i_buf_len);
std::string string_str2hex(const std::string &i_str);
std::string string_str2hex(const char *i_buf, size_t i_buf_len);
std::string string_int2oct(int i_num);

size_t string_bin2raw(const std::string &i_bin, size_t i_buf_len, char *o_buf);
size_t string_hex2raw(const std::string &i_hex, size_t i_buf_len, char *o_buf);

#endif /* string_convert_hpp */
