
#ifndef user_info_macos_h
#define user_info_macos_h

#include <set>
#include <string>

std::set<std::string> user_names();
std::set<std::string> user_names_loginable();
std::set<std::string> user_names_no_underline();

std::set<std::string> user_home_dirs();

#if defined(OS_LINUX)
    std::set<std::string> user_home_dirs_in_users(const std::string &i_key="/home/");
#elif defined(OS_MACOS)
    std::set<std::string> user_home_dirs_in_users(const std::string &i_key="/Users/");
#endif



#endif /* user_info_macos_h */
