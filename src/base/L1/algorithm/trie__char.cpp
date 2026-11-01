
#include <vector>

#include "algorithm/trie__char.hpp"

bool CharTrie::insert(const std::string &i_str)
{
    bool r_bool = true;
    
    do {
        if(i_str.empty()) { r_bool = false; break; }

        CharTrieNodePtr node = this->_root;
        
        for(const auto &character : i_str) {
            if(node->subnodes.end() == node->subnodes.find(character)) {
                node->subnodes[character] = CharTrieNode::create();
            }
            node = node->subnodes[character];
        }
        
        node->end = true;
    } while (false);
    
    return r_bool;
}

bool CharTrie::find(const std::string &i_str) const
{
    uint64_t counter = 0; return this->find(i_str, counter);
}

bool CharTrie::find(const std::string &i_str, uint64_t &o_len) const
{
    CharTrieNodePtr node = _root;
    
    uint64_t loop_counter = 0;
    for(const auto &character : i_str) {
        if(node->is_end()) { break; }

        const auto &it = node->subnodes.find(character);
        if(it == node->subnodes.end()) {
            node = nullptr; break;
        }
        node = it->second; ++loop_counter;
    }
    o_len = loop_counter;

    return (nullptr != node && node->is_end());
}

bool CharTrie::equal(const std::string &i_str, uint64_t &o_len) const
{
    CharTrieNodePtr node = _root;
    
    uint64_t loop_counter = 0;
    for(const auto &character : i_str) {
        ++loop_counter;
        
        const auto &it = node->subnodes.find(character);
        if(it == node->subnodes.end()) {
            node = nullptr; break;
        }
        node = it->second;
    }
    o_len = loop_counter;

    return (nullptr != node && node->is_end());
}



template<typename T> class ReverseLooper
{
public:
    explicit ReverseLooper(const T& i_data) : _data(i_data) {}
public:
    auto begin() const { return std::make_reverse_iterator( _data.end()   ); }
    auto end()   const { return std::make_reverse_iterator( _data.begin() ); }
private:
    const T& _data;
};

bool ReverseCharTrie::insert(const std::string &i_str)
{
    bool r_bool = true;
    
    do {
        if(i_str.empty()) { r_bool = false; break; }

        CharTrieNodePtr node = this->_root;
        
        for(const auto &character : ReverseLooper<std::string>(i_str)) {
            if(node->subnodes.end() == node->subnodes.find(character)) {
                node->subnodes[character] = CharTrieNode::create();
            }
            node = node->subnodes[character];
        }
        
        node->end = true;
    } while (false);
    
    return r_bool;
}

bool ReverseCharTrie::find(const std::string &i_str) const
{
    uint64_t counter = 0; return this->find(i_str, counter);
}

bool ReverseCharTrie::find(const std::string &i_str, uint64_t &o_len) const
{
    CharTrieNodePtr node = _root;

    uint64_t loop_counter = 0;
    for(const auto &character : ReverseLooper<std::string>(i_str)) {
        if(node->is_end()) { break; }

        const auto &it = node->subnodes.find(character);
        if(it == node->subnodes.end()) {
            node = nullptr; break;
        }
        node = it->second; ++loop_counter;
    }
    o_len = loop_counter;

    return (nullptr != node && node->is_end());
}
