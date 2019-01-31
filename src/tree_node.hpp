#ifndef WMDERLAND_TREE_NODE_
#define WMDERLAND_TREE_NODE_

#include <iostream>
#include <vector>

class TreeNode {
public:
    TreeNode(int id);
    ~TreeNode();

    bool IsLeaf() const;
    void AddChild(TreeNode* child);
    void RemoveChild(TreeNode* child);
    void InsertChildAfter(TreeNode* child, TreeNode* ref);
    
    int id() const;
    TreeNode* parent() const;
    void set_parent(TreeNode* parent);
    const std::vector<TreeNode*>& children() const;

    friend std::ostream& operator<< (std::ostream& os, const TreeNode& n);

private:
    int id_;
    TreeNode* parent_;
    std::vector<TreeNode*> children_;
};

#endif
