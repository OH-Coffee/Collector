#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "string/string_convert.hpp"

template std::string string_joint(std::set<std::string>,    char);
template std::string string_joint(std::list<std::string>,   char);
template std::string string_joint(std::vector<std::string>, char);

static inline std::string char2bin(const char i_char)
{
    std::bitset<8> bs(static_cast<char>(i_char));

    return bs.to_string();
}

std::string string_str2bin(const std::string &i_str)
{
    return string_str2bin(i_str.c_str(), i_str.size());
}

std::string string_str2bin(const char *i_buf, size_t i_buf_len)
{
    std::string r_str;

    if(nullptr != i_buf && i_buf_len > 0) {
        for(size_t i = 0; i < i_buf_len; i++) {
            r_str.append( char2bin( i_buf[i] ) );
        }
    }

    return r_str;
}

static inline std::string char2hex(const char i_char)
{
    char buf[3] = { 0, 0 , 0 };

    snprintf(buf, sizeof(buf), "%02X", static_cast<unsigned char>(i_char));

    return buf;
}

std::string string_str2hex(const std::string &i_str)
{
    return string_str2hex(i_str.c_str(), i_str.size());
}

std::string string_str2hex(const char *i_buf, size_t i_buf_len)
{
    std::string r_str;

    if(nullptr != i_buf && i_buf_len > 0) {
        for(size_t i = 0; i < i_buf_len; i++) {
            r_str.append( char2hex( i_buf[i] ) );
        }
    }

    return r_str;
}

std::string string_int2oct(int i_num)
{
    std::string r_string;

    do {
        const bool is_negative = (i_num < 0);

        if(i_num == 0) { r_string = "0"; break; }

        long long num = std::abs(i_num);
        while(num > 0) {
            r_string.push_back('0' + (num % 8)); num /= 8;
        }
        if(is_negative) { r_string.push_back('-'); }

        std::reverse(r_string.begin(), r_string.end());
    } while (false);

    return r_string;
}

size_t string_bin2raw(const std::string &i_bin, size_t i_buf_len, char *o_buf)
{
    size_t r_size = 0;

    for(size_t i = 0; i < i_bin.size(); i += 8) {
        auto val = (char)std::stoul(std::string(i_bin, i, 8), nullptr, 2);

        if(r_size >= i_buf_len) {
            break;
        } else {
            o_buf[r_size++] = val;
        }
    }

    return r_size;
}

size_t string_hex2raw(const std::string &i_hex, size_t i_buf_len, char *o_buf)
{
    size_t r_size = 0;

    for(size_t i = 0; i < i_hex.size(); i += 2) {
        auto val = (char)std::stoul(std::string(i_hex, i, 2), nullptr, 16);

        if(r_size >= i_buf_len) {
            break;
        } else {
            o_buf[r_size++] = val;
        }
    }

    return r_size;
}
