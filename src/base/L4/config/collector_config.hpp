
#ifndef cardinal_conf_manager_hpp
#define cardinal_conf_manager_hpp

#include "config/config_manager.hpp"

class CollectorConfig : public ConfigManager
{
public:
    static CollectorConfig &ins();
public:
    ret_code load(const std::string &i_path) override;
};


#endif /* cardinal_conf_manager_hpp */

