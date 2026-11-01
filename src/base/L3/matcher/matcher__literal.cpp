
#include <cstring>

#include "file/file_api.hpp"
#include "matcher/matcher__literal.hpp"

bool LiteralMatcher::match(const char *i_str) const
{
    if(_match_any)        { return true;                }
    if(_match_nil_suffix) { return !path_suffix(i_str); }
    if(nullptr == i_str)  { return _pattern.empty();    }
    if(_pattern.empty())  { return !strcmp(i_str, "");  }
    
    // 完全匹配
    if(_match_begins_with && _match_ends_with) {
        return ( 0 == strcmp(i_str, LiteralMatcher::_pattern.c_str()) );
    }
    
    const size_t str_len = strlen(i_str);
    const size_t reg_len = LiteralMatcher::_pattern_size;
    
    // 范本长度大于目标长度，无论头匹配、尾匹配还是包含匹配，均为false
    if(reg_len > str_len) { return false; }
    
    if(_match_begins_with) {
        return ( 0 == strncmp(i_str, LiteralMatcher::_pattern.c_str(), reg_len) );
    }

    if(_match_ends_with) {
        return ( 0 == strncmp(i_str + str_len - reg_len, LiteralMatcher::_pattern.c_str(), reg_len) );
    }
    
    return ( nullptr != strstr(i_str, LiteralMatcher::_pattern.c_str()) );
}

bool LiteralMatcher::insert(const std::string &i_pattern)
{
    bool r_bool = true;
    
    do {
        if(i_pattern.empty()) { r_bool = false; break; }
        
        std::string pattern(i_pattern); this->clear();
        
        if("?*" == pattern) { _match_any        = true; break; }
        if("!!" == pattern) { _match_nil_suffix = true; break; }
        
        if('$' == pattern.back())  { _match_ends_with   = true; pattern.pop_back();          }
        if('^' == pattern.front()) { _match_begins_with = true; pattern = pattern.substr(1); }
        
        LiteralMatcher::_pattern        = pattern;
        LiteralMatcher::_pattern_size   = pattern.size();
    } while (false);
    
    return r_bool;
}

