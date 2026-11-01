
#ifndef prefix_tree_hpp
#define prefix_tree_hpp

#include <algorithm>
#include <string>
#include <memory>
#include <unordered_map>

class PrefixTreeNode
{
public:
     PrefixTreeNode() : is_end(false) { }
    ~PrefixTreeNode() = default;
public:
    bool is_end;
public:
    std::unordered_map<std::string, std::shared_ptr<PrefixTreeNode>> sub_node;
};


class PrefixTree
{
public:
     PrefixTree() { _root = std::make_shared<PrefixTreeNode>(); }
    ~PrefixTree() = default;
public:
    bool insert(const std::string &i_str);
    bool find(const std::string &i_str) const;
public:
    template<typename T> bool insert(const T &i_paths);
private:
    std::shared_ptr<PrefixTreeNode> _root;
};


template <typename T>
bool PrefixTree::insert(const T &i_paths)
{
    return std::all_of(std::begin(i_paths), std::end(i_paths), [this](const std::string &i_path){
        return (!i_path.empty() && this->insert(i_path));
    });
}


#endif /* prefix_tree_hpp */

