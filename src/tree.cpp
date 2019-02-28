#include "tree.hpp"
#include <stack>

using std::stack;
using std::vector;
using tiling::Direction;

// In Wmderland, the root node will always exist in a client tree at any given time.
Tree::Tree() : root_(new TreeNode(nullptr)), current_(nullptr) {
    // Initialize a TreeNode with no client associated with it,
    // and set its tiling direction to HORIZONTAL by default.
    root_->set_tiling_direction(Direction::HORIZONTAL);
}

Tree::~Tree() {}


TreeNode* Tree::GetTreeNode(Client* client) const {
    if (!current_) return nullptr;

    TreeNode* ptr = root_.get();
    stack<TreeNode*> s;

    while (ptr) {
        if (!ptr->IsLeaf()) {
            // If ptr has children, push the address of each child
            // onto the stack in reverse order.
            for (int i = ptr->children().size() - 1; i >= 0; i--) {
                s.push(ptr->children()[i]);
            }
        } else {
            if (ptr->client() == client) return ptr;
        }

        // Pop the first item on the stack which is ptr's first child.
        ptr = s.top();
        s.pop();
    }

    return nullptr;
}

vector<TreeNode*> Tree::GetAllLeaves() const {
    vector<TreeNode*> leaves;
    stack<TreeNode*> s;
    TreeNode* ptr = root_.get();
    
    while (ptr) {
        if (!ptr->IsLeaf()) {
            // If ptr has children, push the address of each child
            // onto the stack in reverse order.
            for (int i = ptr->children().size() - 1; i >= 0; i--) {
                s.push(ptr->children()[i]);
            }
        } else {
            leaves.push_back(ptr);
        }

        // Process the next tree node at the top of the stack.
        if (s.empty()) break;
        ptr = s.top();
        s.pop();
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
