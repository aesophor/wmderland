#include <stack>
#include "tree.hpp"

using std::endl;
using std::stack;
using std::vector;
using std::ostream;
using tiling::Direction;

Tree::Tree() : current_(nullptr) {
    // Initialize a TreeNode with no client associated with it,
    // and set its tiling direction to HORIZONTAL by default.
    root_ = new TreeNode(nullptr);
    root_->set_tiling_direction(Direction::HORIZONTAL);
}

Tree::~Tree() {
    delete root_;
}


TreeNode* Tree::GetTreeNode(Client* client) const {
    if (!current_) {
        return nullptr;
    }

    TreeNode* ptr = root_;
    stack<TreeNode*> s;

    while (ptr) {
        if (!ptr->IsLeaf()) {
            // If ptr has children, push the address of each child
            // onto the stack in reverse order.
            for (int i = ptr->children().size() - 1; i >= 0; i--) {
                s.push(ptr->children()[i]);
            }

            // Pop the first item on the stack which is ptr's first child.
            ptr = s.top();
            s.pop();
        } else {
            if (ptr->client() == client) return ptr;
            if (s.empty()) break;
            ptr = s.top();
            s.pop();
        }
    }

    return nullptr;
}

vector<TreeNode*> Tree::GetAllLeaves() const {
    vector<TreeNode*> leaves;
    stack<TreeNode*> s;
    TreeNode* ptr = root_;
    
    while (ptr) {
        if (!ptr->IsLeaf()) {
            // If ptr has children, push the address of each child
            // onto the stack in reverse order.
            for (int i = ptr->children().size() - 1; i >= 0; i--) {
                s.push(ptr->children()[i]);
            }

            // Pop the first item on the stack which is ptr's first child.
            ptr = s.top();
            s.pop();
        } else {
            leaves.push_back(ptr);
            if (s.empty()) break;
            ptr = s.top();
            s.pop();
        }
    }
    return leaves;
}


TreeNode* Tree::root() const {
    return root_;
}

TreeNode* Tree::current() const {
    return current_;
}

void Tree::set_root(TreeNode* root) {
    root_ = root;
}

void Tree::set_current(TreeNode* current) {
    current_ = current;
}
