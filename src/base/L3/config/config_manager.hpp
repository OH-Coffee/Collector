
#ifndef conf_manager_hpp
#define conf_manager_hpp

#include "header/pch.hpp"

#include "jsoncpp/json/json.h"
#include "string/string_split.hpp"

typedef Json::Value (*conf_init_handler)(const std::string &, const Json::Value &);
#define NEW_CONF_INIT_HANDLER(i_name)   static Json::Value i_name(const std::string &i_key, const Json::Value &i_conf)

#define  SET_CONF_FILED(i_item, i_filed, i_value) { (i_item)[(i_filed)] = (i_value); }
#define INIT_CONF_FIELD(i_item, i_filed, i_value)   \
do {                                                \
    if((i_item)[(i_filed)].isNull()) {              \
        (i_item)[(i_filed)] = (i_value);            \
    }                                               \
} while(false)

class ConfigManager
{
public:
    virtual ret_code load(const std::string &i_path);
    virtual ret_code load_by_path(const std::string &i_path);
    virtual ret_code load_by_data(const std::string &i_data);
public:
    std::string to_string(bool i_styled = false);
public:
    int32_t get_int32(const std::string &i_key);
    int64_t get_int64(const std::string &i_key);
public:
    uint32_t get_uint32(const std::string &i_key);
    uint64_t get_uint64(const std::string &i_key);
public:
    bool        get_bool(const std::string &i_key);
    std::string get_string(const std::string &i_key);
public:
    template<typename T> T get_strings(const std::string &i_key);
public:
    template<typename T> void set_value(const std::string &i_key, T i_val, Json::Value *i_start = nullptr, bool i_rewrite = false);
    template<typename T> void set_array(const std::string &i_key, const std::vector<T> &i_array, Json::Value *i_start = nullptr, bool i_rewrite = false);
private:
    Json::Value* key_search(const std::string &i_key);
protected:
    Json::Value  _json_value;
    Json::Reader _json_reader;
};

template<typename T> T ConfigManager::get_strings(const std::string &i_key)
{
    T r_strings;

    do {
        auto val = this->key_search(i_key);

        if(nullptr == val)  { break; }
        if(!val->isArray()) { break; }

        for(auto & it : *val) {
            if(it.isString()) {
                r_strings.push_back(it.asString()); // 后续考虑关联容器
            }
        }
    } while (false);

    return r_strings;
}

template<typename T>
void ConfigManager::set_value(const std::string &i_key, T i_val, Json::Value *i_start, bool i_rewrite)
{
    auto keys = string_split<std::vector<std::string>>(i_key, ".");
    
    Json::Value *val = (nullptr == i_start ? &_json_value : i_start);
    
    for(const auto &key : keys) {
        val = &(*val)[key];
    }
    
    if(val->isNull() || i_rewrite) { *val = i_val; }
}

template<typename T>
void ConfigManager::set_array(const std::string &i_key, const std::vector<T> &i_array, Json::Value *i_start, bool i_rewrite)
{
    auto keys = string_split<std::vector<std::string>>(i_key, ".");
    
    Json::Value *val = (nullptr == i_start ? &_json_value : i_start);
    
    for(const auto &key : keys) {
        val = &(*val)[key];
    }
    
    if(val->isNull() || i_rewrite) {
        *val = Json::Value(Json::arrayValue);
        for(const auto &item : i_array) {
            val->append(item);
        }
    }
}

#endif /* conf_manager_hpp */

