#ifndef string_basic_hpp
#define string_basic_hpp

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "header/errcode.hpp"

ret_code string_copy2(const std::string &i_str, size_t i_max_len,    char *o_buf);
ret_code string_copy2(const std::string &i_str, size_t i_max_len, uint8_t *o_buf);

bool string_is_digit(const std::string &i_str);
bool string_is_digit(const char *i_buf, size_t i_buf_len);

bool string_start_with(const std::string &i_str, char i_sign);
bool string_start_with(const std::string &i_str, const std::string &i_sigh);

std::string string_first_no_space_substr(const std::string &i_str);

std::pair<std::string, std::string> string_pair(const std::string &i_str, const std::string &i_delim, bool i_drop_space);
std::string string_pair_key(const std::string &i_str, const std::string &i_delim, bool i_drop_space = false);
std::string string_pair_val(const std::string &i_str, const std::string &i_delim, bool i_drop_space = false);

#endif /* string_basic_hpp */
