
#ifndef aegis_event_filter_rule_loader_h
#define aegis_event_filter_rule_loader_h

#include <map>

#include "rapidxml/rapidxml.hpp"

#include "rule/struct__group.hpp"

namespace rule {

typedef std::map<uint64_t, GroupPtr>    Rules;
typedef std::shared_ptr<Rules>          RulesPtr;
#define NEW_RULE_GROUP(i_args...) std::make_shared<rule::Group>(i_args)

typedef enum : int64_t
{
    MODE_UNKNOWN = 0,
    MODE_RELOAD,
    MODE_APPEND,
} LoadMode;

class Loader
{
public:
     Loader() = default;
    ~Loader() = default;
public:
    [[nodiscard]] const auto& get_filter_rule_groups() const {
        return this->_rules;
    }   
public:
    ret_code load_by_path(const std::string &i_path, LoadMode);
    ret_code load_by_data(const std::string &i_data, LoadMode);
public:
    ret_code    unload();
    std::string print();
public:
    inline bool enable_default_on_match() const         { return _enable_default_on_match;      }
    inline void enable_default_on_match(bool i_enable)  { _enable_default_on_match = i_enable;  }
private:
    ret_code load(const std::string &i_xml_str, LoadMode);
private:
    ret_code load_rule_groups(rapidxml::xml_node<>*, LoadMode);
private:
    Rules _rules;
private:
    bool _enable_default_on_match{false};
};

} /* namespace aegis_mix */


#endif /* aegis_event_filter_rule_loader_h */
