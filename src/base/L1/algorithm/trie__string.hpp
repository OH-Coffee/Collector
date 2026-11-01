
#ifndef trie__string_h
#define trie__string_h

#include <algorithm>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>

#include "string/string_spliter.hpp"

/* ---------------------------------------------------------------------------------------------------- */

class StringTrie;
class StringTrieNode;


typedef std::shared_ptr<StringTrieNode> StringTrieNodePtr;

/* ---------------------------------------------------------------------------------------------------- */

class StringTrieNode
{
    friend StringTrie;
public:
    template<typename ... T> static StringTrieNodePtr create(T && ... i_args) {
        return std::make_shared<StringTrieNode>(std::forward<T>(i_args)...);
    }
public:
    inline bool is_end() const { return this->end; }
private:
    bool end{false};
private:
    std::unordered_map<std::string, StringTrieNodePtr> subnodes;
};

/* ---------------------------------------------------------------------------------------------------- */

class StringTrie
{
public:
    StringTrie(StringSpliterType i_spliter_type = STRING_FORWARD_SPLITER) {
        _root = StringTrieNode::create();
        
        switch(i_spliter_type) {
            case STRING_FORWARD_SPLITER: { _spliter = StringSpliter::create();        } break;
            case STRING_REVERSE_SPLITER: { _spliter = ReverseStringSpliter::create(); } break;
            default: {} break;
        }
    }
    ~StringTrie() = default;
public:
                         bool insert(const std::string &i_str, const std::string &i_delim);
    template<typename T> bool insert(const T &i_str_container, const std::string &i_delim);
public:
    bool find(const std::string &i_str, const std::string &i_delim) const;
public:
    inline void clear() { _root->subnodes.clear(); }
private:
    StringTrieNodePtr _root;
private:
    StringSpliterPtr  _spliter;
};

template<typename T> bool StringTrie::insert(const T &i_str_container, const std::string &i_delim)
{
    for(const auto &it : i_str_container) {
        if( !StringTrie::insert(it, i_delim) ) { return false; }
    }
    
    return true;
}

/* ---------------------------------------------------------------------------------------------------- */


#endif /* trie__string_h */
