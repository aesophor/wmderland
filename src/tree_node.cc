#include "tree_node.h"
#include <algorithm>

using std::vector;
using std::remove;
using tiling::Direction;

TreeNode::TreeNode(Client* client) : client_(client), tiling_direction_(Direction::UNSPECIFIED) {}

TreeNode::~TreeNode() {}


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
    child->set_parent(this);
}


TreeNode* TreeNode::GetLeftSibling() const {
    const vector<TreeNode*>& siblings = parent_->children();

    if (this == siblings.front()) {
        return nullptr;
    } else {
        ptrdiff_t this_node_idx = find(siblings.begin(), siblings.end(), this) - siblings.begin();
        return siblings[this_node_idx - 1];
    }
}

TreeNode* TreeNode::GetRightSibling() const {
    const vector<TreeNode*>& siblings = parent_->children();

    if (this == siblings.back()) {
        return nullptr;
    } else {
        ptrdiff_t this_node_idx = find(siblings.begin(), siblings.end(), this) - siblings.begin();
        return siblings[this_node_idx + 1];
    }
}


const vector<TreeNode*>& TreeNode::children() const {
    return children_;
}


TreeNode* TreeNode::parent() const {
    return parent_;
}

void TreeNode::set_parent(TreeNode* parent) {
    parent_ = parent;
}


// Get the client associated with this node.
Client* TreeNode::client() const {
    return client_;
}

void TreeNode::set_client(Client* client) {
    client_ = client;
}


Direction TreeNode::tiling_direction() const {
    return tiling_direction_;
}

void TreeNode::set_tiling_direction(Direction tiling_direction) {
    tiling_direction_ = tiling_direction;
}
