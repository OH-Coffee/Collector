#include <algorithm>
#include <cctype>

#include "string/string_transform.hpp"
#include "string/string_split.hpp"

std::string string_reverse(const std::string &i_str, const std::string &i_delim)
{
    std::string r_data;

    do {
        const auto items = string_split<std::vector<std::string>>(i_str, i_delim);
        if(!items.empty()) {
            r_data.reserve(i_str.size());
        }

        for(auto it = items.rbegin(); it != items.rend(); ++it) {
            if(!it->empty()) { r_data.append(*it).append(i_delim); }
        }

        const auto  data_size = r_data.size();
        const auto delim_size = i_delim.size();

        if(data_size > delim_size) {
            r_data.erase(data_size - delim_size, delim_size);
        }
    } while (false);

    return r_data;
}

std::string string_replace(const std::string &i_subject, const std::string &i_target, const std::string &i_replacement)
{
    std::string ret_str(i_subject);

    size_t offset = 0;
    while (std::string::npos != (offset = ret_str.find(i_target, offset))) {
        ret_str.replace(offset, i_target.size(), i_replacement); offset += i_replacement.size();
    }

    return ret_str;
}

std::set<std::string> string_replace(const std::string &i_subject, const std::string &i_target, std::set<std::string> i_replacements)
{
    std::set<std::string> ret_strings;

    auto site = i_subject.find(i_target);
    if(site != std::string::npos) {
        for(const auto &replacement : i_replacements) {
            ret_strings.insert(std::string(i_subject).replace(site, i_target.size(), replacement));
        }
    }

    return ret_strings;
}

std::set<std::string> string_replace_recursion(const std::string &i_subject, const std::string &i_target, std::set<std::string> i_replacements)
{
    std::set<std::string> ret_strings;

    auto strings = string_replace(i_subject, i_target, i_replacements);
    for(const auto &str : strings) {
        auto tmp_strings = string_replace_recursion(str, i_target, i_replacements);

        if(tmp_strings.empty()) {
            ret_strings.insert(str);
        } else {
            ret_strings.insert(tmp_strings.begin(), tmp_strings.end());
        }
    }

    return ret_strings;
}

std::string string_to_lower(const std::string &i_str)
{
    std::string ret_str(i_str);

    for(char &it : ret_str) {
        it = static_cast<char>(std::tolower(it));
    }

    return ret_str;
}

std::string string_to_upper(const std::string &i_str)
{
    std::string ret_str(i_str);

    for(char &it : ret_str) {
        it = static_cast<char>(std::toupper(it));
    }

    return ret_str;
}

std::string string_drop_quotes(const std::string &i_str)
{
    std::string ret_str(i_str);

    do {
        if(ret_str.empty()) { break; }

        const char front = i_str.front();
        const char back  = i_str.back();

        if('"' == back  || '\'' == back)  { ret_str.pop_back();                 }
        if('"' == front || '\'' == front) { ret_str.assign(ret_str.substr(1));  }
    } while (false);

    return ret_str;
}

std::string string_add_prefix(const std::string &i_str, const std::string &i_prefix)
{
    return (i_prefix + i_str);
}

std::string string_add_suffix(const std::string &i_str, const std::string &i_suffix)
{
    return (i_str + i_suffix);
}

std::string string_surround(const std::string &i_str, const std::string &i_prefix, const std::string &i_suffix)
{
    return (i_prefix + i_str + i_suffix);
}
