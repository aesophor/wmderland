#include "tree.h"

#include <stack>

using std::stack;
using std::vector;
using std::unordered_map;
using tiling::Direction;

unordered_map<Client*, TreeNode*> TreeNode::mapper_;

Tree::Tree() : root_(new TreeNode(nullptr)), current_(nullptr) {
  // NOTE: In Wmderland, the root node will always exist in a client tree
  // at any given time.
  
  // Initialize a TreeNode with no client associated with it,
  // and set its tiling direction to HORIZONTAL by default.
  root_->set_tiling_direction(Direction::HORIZONTAL);
}


TreeNode* Tree::GetTreeNode(Client* client) const {
  if (TreeNode::mapper_.find(client) != TreeNode::mapper_.end()) {
    return TreeNode::mapper_.at(client);
  }
  return nullptr;
}

vector<TreeNode*> Tree::GetAllLeaves() const {
  vector<TreeNode*> leaves;
  stack<TreeNode*> st;
  st.push(root_.get());

  while (!st.empty()) {
    TreeNode* node = st.top();
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


TreeNode* Tree::root() const {
  return root_.get();
}

TreeNode* Tree::current() const {
  return current_;
}

void Tree::set_current(TreeNode* current) {
  current_ = current;
}
