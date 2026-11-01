
#include <pwd.h>
#include <string.h>
#include <sys/types.h>

#include "user/user_info.hpp"


std::set<std::string> user_names()
{
    std::set<std::string> ret_set;
    
    struct passwd *pwd;
    while (NULL != (pwd = getpwent())) {
        if(NULL != pwd->pw_name) { ret_set.insert(pwd->pw_name); }
    }
    endpwent();
    
    return ret_set;
}

std::set<std::string> user_names_loginable()
{
    std::set<std::string> ret_set;
    
    struct passwd *pwd;
    while (NULL != (pwd = getpwent())) {
        if(NULL != pwd->pw_name
        &&(0 != strcmp(pwd->pw_shell, "/usr/bin/false")))
        {
            ret_set.insert(pwd->pw_name);
        }
    }
    endpwent();
    
    return ret_set;
}
std::set<std::string> user_names_no_underline()
{
    std::set<std::string> ret_set;
    
    struct passwd *pwd;
    while (NULL != (pwd = getpwent())) {
        if(NULL != pwd->pw_name && pwd->pw_name[0] != '_') { ret_set.insert(pwd->pw_name); }
    }
    endpwent();
    
    return ret_set;
}

std::set<std::string> user_home_dirs()
{
    std::set<std::string> ret_set;
    
    struct passwd *pwd;
    while (NULL != (pwd = getpwent())) {
        if(pwd->pw_dir) { ret_set.insert(pwd->pw_dir); }
    }
    endpwent();
    
    return ret_set;
}

std::set<std::string> user_home_dirs_in_users(const std::string &i_key)
{
    std::set<std::string> ret_set;
    
    struct passwd *pwd;
    while (nullptr != (pwd = getpwent())) {
        if(pwd->pw_dir && 0 == strncmp(pwd->pw_dir, i_key.c_str(), i_key.size())) {
            ret_set.insert(pwd->pw_dir);
        }
    }
    endpwent();
    
    return ret_set;
}
