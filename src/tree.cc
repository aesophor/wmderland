// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "tree.h"

#include <stack>
#include <algorithm>

using std::stack;
using std::vector;
using std::unordered_map;

namespace wmderland {

unordered_map<Client*, Tree::Node*> Tree::Node::mapper_;

Tree::Tree() : root_node_(new Tree::Node(nullptr)), current_node_() {
  // NOTE: In Wmderland, the root node will always exist in a client tree
  // at any given time.
  
  // Initialize a Tree::Node with no client associated with it,
  // and set its tiling direction to HORIZONTAL by default.
  root_node_->set_tiling_direction(TilingDirection::HORIZONTAL);
}

Tree::~Tree() {
  TreeDfsCleanUp(root_node_);
}

void Tree::TreeDfsCleanUp(Tree::Node* node) const {
  if (!node) {
    return;
  }

  for (const auto child : node->children()) {
    TreeDfsCleanUp(child);
  }
  delete node;
}


Tree::Node* Tree::GetTreeNode(Client* client) const {
  if (Tree::Node::mapper_.find(client) != Tree::Node::mapper_.end()) {
    return Tree::Node::mapper_.at(client);
  }
  return nullptr;
}

vector<Tree::Node*> Tree::GetAllLeaves() const {
  vector<Tree::Node*> leaves;
  stack<Tree::Node*> st;
  st.push(root_node_);

  while (!st.empty()) {
    Tree::Node* node = st.top();
    st.pop();
    
    // If this node is a leaf, add it to the leaf vector.
    if (node->children().empty()) {
      leaves.push_back(node);
    }

    // Push all children onto the stack in reverse order (if any).
    for (int i = node->children().size() - 1; i >= 0; i--) {
      st.push(node->children().at(i));
    }
  }
  return leaves;
}


Tree::Node* Tree::root_node() const {
  return root_node_;
}

Tree::Node* Tree::current_node() const {
  return current_node_;
}

void Tree::set_current_node(Tree::Node* node) {
  current_node_ = node;
}



Tree::Node::Node(Client* client)
    : client_(client),
      tiling_direction_(TilingDirection::UNSPECIFIED) {
  Tree::Node::mapper_[client] = this;
}

Tree::Node::~Node() {
  Tree::Node::mapper_.erase(client_);
}


void Tree::Node::AddChild(Tree::Node* child) {
  children_.push_back(child);
  child->set_parent(this);
}

void Tree::Node::RemoveChild(Tree::Node* child) {
  children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
  child->set_parent(nullptr);
}

void Tree::Node::InsertChildAfter(Tree::Node* child, Tree::Node* ref) {
  ptrdiff_t ref_idx = find(children_.begin(), children_.end(), ref) - children_.begin();
  children_.insert(children_.begin() + ref_idx + 1, child);
  child->set_parent(this);
}


Tree::Node* Tree::Node::GetLeftSibling() const {
  const vector<Tree::Node*>& siblings = parent_->children();

  if (this == siblings.front()) {
    return nullptr;
  } else {
    ptrdiff_t this_node_idx = find(siblings.begin(), siblings.end(), this) - siblings.begin();
    return siblings[this_node_idx - 1];
  }
}

Tree::Node* Tree::Node::GetRightSibling() const {
  const vector<Tree::Node*>& siblings = parent_->children();

  if (this == siblings.back()) {
    return nullptr;
  } else {
    ptrdiff_t this_node_idx = find(siblings.begin(), siblings.end(), this) - siblings.begin();
    return siblings[this_node_idx + 1];
  }
}


const vector<Tree::Node*>& Tree::Node::children() const {
  return children_;
}


Tree::Node* Tree::Node::parent() const {
  return parent_;
}

void Tree::Node::set_parent(Tree::Node* parent) {
  parent_ = parent;
}


// Get the client associated with this node.
Client* Tree::Node::client() const {
  return client_;
}

void Tree::Node::set_client(Client* client) {
  client_ = client;
}


TilingDirection Tree::Node::tiling_direction() const {
  return tiling_direction_;
}

void Tree::Node::set_tiling_direction(TilingDirection tiling_direction) {
  tiling_direction_ = tiling_direction;
}

bool Tree::Node::leaf() const {
  return children_.empty();
}

} // namespace wmderland
