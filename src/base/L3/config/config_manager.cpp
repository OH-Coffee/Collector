
#include "file/file_api.hpp"

#include "config/config_manager.hpp"

ret_code ConfigManager::load(const std::string &i_path)
{
    return load_by_path(i_path);
}

ret_code ConfigManager::load_by_path(const std::string &i_path)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        std::string json_str = file_read_all_s(i_path);
        if(json_str.empty()) {
            r_code = RC_FILE_READ_ERR; break;
        }
        
        r_code = load_by_data(json_str);
    } while (false);
    
    return r_code;
}

ret_code ConfigManager::load_by_data(const std::string &i_data)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(!_json_reader.parse(i_data, _json_value, false)) {
            r_code = RC_JSON_SERIALIZE_ERR; break;
        }
        
        r_code = RC_OK;
    } while (false);
    
    return r_code;
}

std::string ConfigManager::to_string(bool i_styled)
{
    std::shared_ptr<Json::Writer> writer = nullptr;
    if(!i_styled) {
        writer = std::make_shared<Json::FastWriter>();
    } else {
        writer = std::make_shared<Json::StyledWriter>();
    }

    return writer->write(_json_value);
}

Json::Value* ConfigManager::key_search(const std::string &i_key)
{
    auto keys = string_split<std::vector<std::string>>(i_key, ".");
    
    Json::Value *json_value = &_json_value;
    for(const auto &key : keys) {
        if(nullptr == json_value || !json_value->isObject() || !json_value->isMember(key)) {
            return nullptr;
        }
        json_value = &(*json_value)[key];
    }
    
    return json_value;
}

int32_t ConfigManager::get_int32(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isInt()) {
        return val->asInt();
    }
    return -1;
}

int64_t ConfigManager::get_int64(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isInt64()) {
        return val->asInt64();
    }
    return -1;
}

uint32_t ConfigManager::get_uint32(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isUInt()) {
        return val->asUInt();
    }
    return 0;
}

uint64_t ConfigManager::get_uint64(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isUInt64()) {
        return val->asUInt64();
    }
    return 0;
}

bool ConfigManager::get_bool(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isBool()) {
        return val->asBool();
    }
    return false;
}

std::string ConfigManager::get_string(const std::string &i_key)
{
    const auto val = key_search(i_key);
    
    if(nullptr != val && val->isString()) {
        return val->asString();
    }
    return "";
}
