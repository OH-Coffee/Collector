
#include <vector>

#include "algorithm/prefix_tree.hpp"
#include "string/string_split.hpp"

bool PrefixTree::insert(const std::string &i_path)
{
    bool r_bool = true;
    
    do {
        auto items = string_split<std::vector<std::string>>(i_path, "/", true);
        if(items.empty()) { r_bool = false; break; }

        std::shared_ptr<PrefixTreeNode> node = _root;
        
        for(const auto & item : items) {
            if(node->sub_node.end() == node->sub_node.find(item)) {
                node->sub_node[item] = std::make_shared<PrefixTreeNode>();
            }
            node = node->sub_node[item];
        }
        node->is_end = true;
    } while (false);
    
    return r_bool;
}

bool PrefixTree::find(const std::string &i_path) const
{
    bool r_bool = false;
    
    do {
        std::shared_ptr<PrefixTreeNode> node = _root;
        
        string_split(i_path, "/", [&node](const std::string_view &i_str) -> bool {
            bool r_continue = true;
            
            do {
                if(i_str.empty()) { r_continue = true;  break; }
                if(node->is_end)  { r_continue = false; break; }
                
                const auto &it = node->sub_node.find( std::string(i_str) );
                if(it == node->sub_node.end()) {
                    node = nullptr; r_continue = false; break;
                }
                node = it->second;
            } while (false);
            
            return r_continue;
        });
        
        r_bool = (node != nullptr && node->is_end);
    } while (false);
    
    return r_bool;
}

