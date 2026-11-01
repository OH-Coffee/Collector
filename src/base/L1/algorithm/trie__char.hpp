
#ifndef trie__char_h
#define trie__char_h

#include <algorithm>
#include <string>
#include <memory>
#include <unordered_map>
#include <utility>

/* ---------------------------------------------------------------------------------------------------- */

class CharTrieNode;

class CharTrie;
class ReverseCharTrie;

typedef std::shared_ptr<CharTrieNode> CharTrieNodePtr;

/* ---------------------------------------------------------------------------------------------------- */

class CharTrieNode
{
    friend CharTrie;
    friend ReverseCharTrie;
public:
    template<typename ... T> static CharTrieNodePtr create(T && ... i_args) {
        return std::make_shared<CharTrieNode>(std::forward<T>(i_args)...);
    }
public:
    inline bool is_end() const { return this->end; }
private:
    bool end{false};
private:
    std::unordered_map<char, CharTrieNodePtr> subnodes{};
};

/* ---------------------------------------------------------------------------------------------------- */

class CharTrie
{
public:
     CharTrie() { _root = CharTrieNode::create(); }
    ~CharTrie() = default;
public:
                         bool insert(const std::string &i_str);
    template<typename T> bool insert(const T &i_str_container);
public:
    bool find(const std::string &i_str)                  const;
    bool find(const std::string &i_str, uint64_t &o_len) const;
public:
    bool equal(const std::string &i_str, uint64_t &o_len) const;
public:
    inline void clear() { _root->subnodes.clear(); }
private:
    CharTrieNodePtr _root;
};

template<typename T> bool CharTrie::insert(const T &i_str_container)
{
    for(const auto &str : i_str_container) {
        if( !CharTrie::insert(str) ) { return false; }
    }
    
    return true;
}

/* ---------------------------------------------------------------------------------------------------- */

class ReverseCharTrie
{
public:
     ReverseCharTrie() { _root = CharTrieNode::create(); }
    ~ReverseCharTrie() = default;
public:
                         bool insert(const std::string &i_str);
    template<typename T> bool insert(const T &i_str_container);
public:
    bool find(const std::string &i_str)                  const;
    bool find(const std::string &i_str, uint64_t &o_len) const;
public:
    inline void clear() { _root->subnodes.clear(); }
private:
    CharTrieNodePtr _root;
};

template<typename T> bool ReverseCharTrie::insert(const T &i_str_container)
{
    for(const auto &str : i_str_container) {
        if( !ReverseCharTrie::insert(str) ) { return false; }
    }
    
    return true;
}

/* ---------------------------------------------------------------------------------------------------- */



#endif /* trie__char_h */
