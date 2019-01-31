#include "tree_node.hpp"
#include <algorithm>
#include <iostream>

using std::vector;
using std::remove;

TreeNode::TreeNode(int id) : id_(id) {

}

TreeNode::~TreeNode() {

}


bool TreeNode::IsLeaf() const {
    return children_.empty();
}

void TreeNode::AddChild(TreeNode* child) {
    children_.push_back(child);
    child->set_parent(this);
}

void TreeNode::RemoveChild(TreeNode* child) {
    children_.erase(remove(children_.begin(), children_.end(), child), children_.end());
    child->set_parent(nullptr);
}

void TreeNode::InsertChildAfter(TreeNode* child, TreeNode* ref) {
    ptrdiff_t ref_idx = find(children_.begin(), children_.end(), ref) - children_.begin();
    children_.insert(children_.begin() + ref_idx + 1, child);
}


int TreeNode::id() const {
    return id_;
}

TreeNode* TreeNode::parent() const {
    return parent_;
}

void TreeNode::set_parent(TreeNode* parent) {
    parent_ = parent;
}

const std::vector<TreeNode*>& TreeNode::children() const {
    return children_;
}

std::ostream& operator<< (std::ostream& os, const TreeNode& n) {
    return os << n.id();
}

/*
int main() {
    vector<TreeNode*> nodes;
    for (int i = 0; i < 5; i++) {
        nodes.push_back(new TreeNode(i));
    }

    // Testing my pathetic tree node api Q_Q
    nodes[0]->AddChild(nodes[1]);
    nodes[0]->AddChild(nodes[2]);
    nodes[0]->InsertChildAfter(nodes[3], nodes[1]);
    nodes[0]->AddChild(nodes[4]);
    nodes[0]->RemoveChild(nodes[2]);
    nodes[0]->AddChild(nodes[2]);

    for (const auto& child : nodes[0]->children()) {
        std::cout << *child << ",";
    }
    std::cout << std::endl;
}
*/
