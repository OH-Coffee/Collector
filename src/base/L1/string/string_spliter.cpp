
#include "string/string_spliter.hpp"


void StringSpliter::split(const std::string_view &i_str,
                          const std::string_view &i_delim,
                          const std::function<bool (const std::string_view &)> &i_callback)
{
    return string_split(i_str, i_delim, i_callback);
}

void StringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::set<std::string> &o_data)
{
    o_data = string_split< std::set<std::string> >(i_str, i_delim, i_drop_empty);
}

void StringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::list<std::string> &o_data)
{
    o_data = string_split< std::list<std::string> >(i_str, i_delim, i_drop_empty);
}

void StringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::vector<std::string> &o_data)
{
    o_data = string_split< std::vector<std::string> >(i_str, i_delim, i_drop_empty);
}





void ReverseStringSpliter::split(const std::string_view &i_str,
                                 const std::string_view &i_delim,
                                 const std::function<bool (const std::string_view &)> &i_callback)
{
    return string_rsplit(i_str, i_delim, i_callback);
}

void ReverseStringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::set<std::string> &o_data)
{
    o_data = string_rsplit< std::set<std::string> >(i_str, i_delim, i_drop_empty);
}

void ReverseStringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::list<std::string> &o_data)
{
    o_data = string_rsplit< std::list<std::string> >(i_str, i_delim, i_drop_empty);
}

void ReverseStringSpliter::split(const std::string_view &i_str, const std::string_view &i_delim, bool i_drop_empty, std::vector<std::string> &o_data)
{
    o_data = string_rsplit< std::vector<std::string> >(i_str, i_delim, i_drop_empty);
}
