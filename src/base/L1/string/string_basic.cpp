#include <algorithm>
#include <cctype>
#include <cstring>

#include "string/string_basic.hpp"

ret_code string_copy2(const std::string &i_str, size_t i_max_len, char *o_buf)
{
    return string_copy2(i_str, i_max_len, (uint8_t *)o_buf);
}

ret_code string_copy2(const std::string &i_str, size_t i_max_len, uint8_t *o_buf)
{
    ret_code r_code = RC_UNKNOWN;

    do {
        if(nullptr == o_buf) { r_code = RC_ARG_ERR; break; }

        memcpy(o_buf, i_str.c_str(), std::min(i_str.size(), i_max_len));

        r_code = RC_OK;
    } while (false);

    return r_code;
}

bool string_is_digit(const std::string &i_str)
{
    return string_is_digit(i_str.c_str(), i_str.size());
}

bool string_is_digit(const char *i_buf, size_t i_buf_len)
{
    bool r_bool = true;

    do {
        if(nullptr == i_buf) { r_bool = false; break; }

        for(size_t i = 0; i < i_buf_len; ++i) {
            if(!isdigit(i_buf[i])) { r_bool = false; break; }
        }
    } while (false);

    return r_bool;
}

bool string_start_with(const std::string &i_str, const char i_sign)
{
    bool r_bool = true;

    do {
        const auto site = i_str.find(i_sign);
        if(std::string::npos == site) {
            r_bool = false; break;
        }

        for(uint64_t i = 0; i < site; i++) {
            if(!isspace(i_str[i])) {
                r_bool = false; break;
            }
        }
    } while (false);

    return r_bool;
}

bool string_start_with(const std::string &i_str, const std::string &i_sigh)
{
    return (0 == i_str.compare(0, i_sigh.size(), i_sigh));
}

std::string string_first_no_space_substr(const std::string &i_str)
{
    if(i_str.empty()) { return ""; }

    size_t offset_s = 0, offset_e = 0;

    while (offset_s < i_str.size() &&  isspace(i_str[offset_s])) { ++offset_s; }
    offset_e = offset_s;
    while (offset_e < i_str.size() && !isspace(i_str[offset_e])) { ++offset_e; }

    return i_str.substr(offset_s, offset_e - offset_s);
}

std::pair<std::string, std::string> string_pair(const std::string &i_str, const std::string &i_delim, bool i_drop_space)
{
    std::pair<std::string, std::string> ret_pair{ i_str, ""};

    auto offset = i_str.find(i_delim);
    if(std::string::npos != offset) {
        std::string key, val;

        key = i_str.substr(0, offset);
        key = !i_drop_space ? key : string_first_no_space_substr(key);

        offset += i_delim.size();

        if(offset < i_str.size()) {
            val = i_str.substr(offset);
            val = !i_drop_space ? val : string_first_no_space_substr(val);
        }

        ret_pair = { key, val };
    }

    return ret_pair;
}

std::string string_pair_key(const std::string &i_str, const std::string &i_delim, const bool i_drop_space)
{
    return string_pair(i_str, i_delim, i_drop_space).first;
}

std::string string_pair_val(const std::string &i_str, const std::string &i_delim, const bool i_drop_space)
{
    return string_pair(i_str, i_delim, i_drop_space).second;
}
