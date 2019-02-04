#ifndef WMDERLAND_TREE_NODE_HPP_
#define WMDERLAND_TREE_NODE_HPP_

#include <vector>
#include "util.hpp"

class Client;

class TreeNode {
public:
    TreeNode(Client* client);
    virtual ~TreeNode();

    bool IsLeaf() const;
    void AddChild(TreeNode* child);
    void RemoveChild(TreeNode* child);
    void InsertChildAfter(TreeNode* child, TreeNode* ref);

    TreeNode* GetLeftSibling() const;
    TreeNode* GetRightSibling() const;

    const std::vector<TreeNode*>& children() const;
    TreeNode* parent() const;
    void set_parent(TreeNode* parent);

    Client* client() const;
    void set_client(Client* client);

    tiling::Direction tiling_direction() const;
    void set_tiling_direction(tiling::Direction tiling_direction);

private:
    std::vector<TreeNode*> children_;
    TreeNode* parent_;

    Client* client_;
    tiling::Direction tiling_direction_;
};

#endif
