
#ifndef trie_h
#define trie_h

#include <list>
#include <memory>

#include "algorithm/trie__char.hpp"
#include "algorithm/trie__string.hpp"


template <typename T> class Trie;



template <typename T> class TrieNode
{
    friend Trie<T>;
public:
    inline bool is_end() { return this->end; }
public:
    inline void new_node(T i_key) {
        this->subnodes[i_key] = std::make_shared< TrieNode<T> >();
    }
private:
    bool end{false};
private:
    std::unordered_map< T, std::shared_ptr< TrieNode<T>> > subnodes{};
};
template <typename T> using TrieNodePtr = std::shared_ptr< TrieNode<T> >;



template <typename T> class Trie
{
public:
     Trie() { _root = std::make_shared< TrieNode<T> >(); }
    ~Trie() = default;
public:
    template <typename Iterator> void insert(Iterator i_begin, Iterator i_end) {
        TrieNodePtr<T> node = _root;
        
        for(auto it = i_begin; it != i_end; ++it) {
            if(node->subnodes.end() == node->subnodes.find(*it)) {
                node->new_node(*it);
            }
            node = node->subnodes[*it];
        }
        
        node->end = true;
    }
public:
    inline const TrieNodePtr<T> find(const T & i_object, const TrieNodePtr<T> & i_start) {
              auto &node_ptr = (i_start == nullptr) ? _root : i_start;
        const auto &sub_node = node_ptr->subnodes.find(i_object);
        
        return (sub_node != node_ptr->subnodes.end()) ? sub_node->second : nullptr;
    };
private:
    TrieNodePtr<T> _root;
};


#endif /* trie_h */
