
#include <vector>

#include "algorithm/trie__string.hpp"
#include "string/string_split.hpp"

bool StringTrie::insert(const std::string &i_str, const std::string &i_delim)
{
    bool r_bool = true;
    
    do {
        std::vector<std::string> items;
        
        _spliter->split(i_str, i_delim, true, items);
        if(items.empty()) {
            r_bool = false; break;
        }

        StringTrieNodePtr node = this->_root;
        
        for(const auto &it : items) {
            if(node->subnodes.end() == node->subnodes.find(it)) {
                node->subnodes[it] = StringTrieNode::create();
            }
            node = node->subnodes[it];
        }
        
        node->end = true;
    } while (false);
    
    return r_bool;
}

bool StringTrie::find(const std::string &i_str, const std::string &i_delim) const
{
    bool r_bool = false;
    
    do {
        StringTrieNodePtr node = this->_root;
        
        _spliter->split(i_str, i_delim, [&node](const std::string_view &i_item) -> bool {
            bool r_continue = true;
            
            do {
                if(i_item.empty()) { r_continue = true;  break; }
                if(node->is_end()) { r_continue = false; break; }
                
                const auto &it = node->subnodes.find( std::string(i_item) );
                if(it == node->subnodes.end()) {
                    node = nullptr; r_continue = false; break;
                }
                node = it->second;
            } while (false);
            
            return r_continue;
        });
        
        r_bool = (node != nullptr && node->is_end());
    } while (false);
    
    return r_bool;
}

