
#include <iterator>

#include "file/file_api.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "string/string_split.hpp"
#include "struct__rule.hpp"
#include "rule/loader.hpp"

namespace rule { /* namespace aegis_mix */

namespace {

void append(rapidxml::xml_document<>& io_doc,
                      rapidxml::xml_node<>* i_node,
                      const char* i_name,
                      const std::string& i_value)
{
    i_node->append_attribute(io_doc.allocate_attribute(
                             io_doc.allocate_string(i_name),
                             io_doc.allocate_string(i_value.c_str())));
}

} // namespace

ret_code Loader::load_by_data(const std::string &i_data, LoadMode i_mode)
{
    return this->load(i_data, i_mode);
}
    
ret_code Loader::load_by_path(const std::string &i_path, LoadMode i_mode)
{
    return this->load(file_read_all_s(i_path), i_mode);
}

ret_code Loader::load(const std::string &i_xml_str, LoadMode i_mode)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        rapidxml::xml_document<> doc;
        std::string xml_buffer = i_xml_str;
        
        try {
            doc.parse<0>(xml_buffer.data());
        } catch (...) {
            r_code = RC_XML_PARSE_ERR; break;
        }
        
        rapidxml::xml_node<>* node_filter_rules = doc.first_node("FilterRules");
        if(nullptr == node_filter_rules) {
            r_code = RC_XML_PARSE_ERR; break;
        }
        
        r_code = load_rule_groups(node_filter_rules, i_mode);
        
    } while (false);
    
    return r_code;
}

ret_code Loader::unload()
{
    _rules.clear();
    
    return RC_OK;
}

std::string Loader::print()
{
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<>* root = doc.allocate_node(rapidxml::node_element, doc.allocate_string("FilterRules"));
    doc.append_node(root);

    for(const auto& [event, group] : _rules) {
        if(nullptr == group) {
            continue;
        }

        rapidxml::xml_node<>* group_node = doc.allocate_node(rapidxml::node_element, doc.allocate_string("EventRuleGroup"));
        append(doc, group_node, "ID", std::to_string(group->id));
        append(doc, group_node, "EventID", std::to_string(group->event));
        root->append_node(group_node);

        for(const auto& rule : group->rules) {
            if(nullptr == rule) {
                continue;
            }

            rapidxml::xml_node<>* rule_node = doc.allocate_node(rapidxml::node_element, doc.allocate_string("Rule"));
            append(doc, rule_node, "ID",           std::to_string(rule->id));
            append(doc, rule_node, "Action",       std::to_string(rule->action));
            append(doc, rule_node, "Relationship", std::to_string(rule->relationship));
            group_node->append_node(rule_node);

            for(const auto& condition : rule->conditions) {
                if(nullptr == condition) {
                    continue;
                }

                rapidxml::xml_node<>* condition_node = doc.allocate_node(rapidxml::node_element, doc.allocate_string("Condition"));
                append(doc, condition_node, "ID",       std::to_string(condition->id));
                append(doc, condition_node, "Object",   std::to_string(condition->object));
                append(doc, condition_node, "Field",    std::to_string(condition->feild));
                append(doc, condition_node, "Matcher",  std::to_string(condition->matcher));

                std::string pattern_text;
                for(const auto& pattern : condition->patterns) {
                    pattern_text.append(pattern).append("|");
                }
                if(!pattern_text.empty() && pattern_text.back() == '|') {
                    pattern_text.pop_back();
                }
                condition_node->value(doc.allocate_string(pattern_text.c_str()), pattern_text.size());
                
                rule_node->append_node(condition_node);
            }
        }
    }

    std::string xml;
    rapidxml::print(std::back_inserter(xml), *root);
    return xml;
}

#define ATTR(i_node, i_key) ( i_node->first_attribute(i_key) )
#define ATOS(i_node, i_key) ( (nullptr != ATTR(i_node, i_key)) ? ATTR(i_node, i_key)->value() : "" )
#define ATOL(i_node, i_key) ( strtol(ATOS(i_node, i_key), nullptr, 0) )

ret_code Loader::load_rule_groups(rapidxml::xml_node<> *i_node, LoadMode i_mode)
{
    ret_code r_code = RC_UNKNOWN;
    
    do {
        if(nullptr == i_node) { break; }
        
        _rules.clear();
        
        rapidxml::xml_node<>* node_group = i_node->first_node("EventRuleGroup");
        for( ; node_group; node_group = node_group->next_sibling("EventRuleGroup")) {
            GroupPtr group = Group::create();
            {
                group->id    = ATOL(node_group, "ID");
                group->event = ATOL(node_group, "EventID");
            }
            
            rapidxml::xml_node<>* node_rule = node_group->first_node("Rule");
            for( ; node_rule; node_rule = node_rule->next_sibling("Rule")) {
                RulePtr rule = Rule::create();
                {
                    rule->id            = ATOL(node_rule, "ID");
                    rule->action        = ATOL(node_rule, "Action");
                    rule->relationship  = ATOL(node_rule, "Relationship");
                }

                ConditionPtr            last_condition = nullptr;
                rapidxml::xml_node<>*   node_condition = node_rule->first_node("Condition");
                
                for( ; node_condition; node_condition = node_condition->next_sibling("Condition")) {
                    ConditionPtr condition = Condition::create();
                    {
                        condition->id       = ATOL(node_condition, "ID");
                        condition->object   = ATOL(node_condition, "Object");
                        condition->feild    = (nullptr != ATTR(node_condition, "Field"))
                                              ? ATOL(node_condition, "Field")
                                              : ATOL(node_condition, "Feild");
                        condition->matcher  = ATOL(node_condition, "Matcher");
                        condition->patterns = string_split<decltype(condition->patterns)>(node_condition->value(), "|");
                    }
                    
                    bool same_condition = (nullptr != last_condition
                                          && condition->id      == last_condition->id
                                          && condition->matcher == last_condition->matcher
                                          && condition->feild   == last_condition->feild
                                          && condition->object  == last_condition->object);
                    
                    if(!same_condition) {
                        rule->conditions.emplace_back(last_condition = condition);
                    } else {
                        last_condition->patterns.insert(last_condition->patterns.end(),
                                                        condition->patterns.begin(),
                                                        condition->patterns.end());
                    }
                }
                rule->conditions.sort([](const auto &a, const auto &b){
                    return (a->id < b->id);
                });
                
                group->rules.emplace_back(rule);
            }
            group->rules.sort([](const auto &a, const auto &b){
                return (a->id < b->id);
            });

            _rules[group->event] = group;
        }
        
        r_code = RC_OK;
    } while (false);
    
    return r_code;
}
    
#undef ATTR
#undef ATOS
#undef ATOL


} /* namespace aegis_mix */
