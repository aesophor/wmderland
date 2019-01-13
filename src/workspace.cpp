#include <glog/logging.h>
#include <algorithm>
#include <stack>
#include "workspace.hpp"
#include "config.hpp"
#include "tiling.hpp"
#include "util.hpp"

using std::pair;
using std::stack;
using std::vector;
using std::remove_if;
using tiling::Direction;

Workspace::Workspace(Display* dpy, Window root_window, int id) : 
    dpy_(dpy), root_window_(root_window), client_tree_(new Tree()), id_(id), is_fullscreen_(false) {

}

Workspace::~Workspace() {
    delete client_tree_;
}


bool Workspace::Has(Window w) {
    return GetClient(w) != nullptr;
}

void Workspace::Add(Window w, bool is_floating) {
    Client* c = new Client(dpy_, w, this);
    c->set_floating(is_floating);

    TreeNode* new_node = new TreeNode(c);

    if (client_tree_->root()->children().empty()) {
        // If there's no any window at all, add the node
        // as the root's child.
        client_tree_->root()->AddChild(new_node);
    } else if (client_tree_->current()->tiling_direction() == Direction::UNSPECIFIED) {
        // If the user has not specified any tiling direction
        // on current node, then add the new node as its brother.
        TreeNode* current_node = client_tree_->current();
        if (current_node == current_node->parent()->children().back()) {
            current_node->parent()->AddChild(new_node);
        } else {
            current_node->parent()->InsertChild(new_node, current_node);
        }
    } else {
        // If the user has specified a tiling direction on
        // current node, then set current node as an internal node,
        // add the original current node as this internal node's child
        // and add the new node as this internal node's another child.
        TreeNode* current_node = client_tree_->current();
        current_node->AddChild(new TreeNode(current_node->client()));
        current_node->AddChild(new_node);
        current_node->set_client(nullptr);
    }

    client_tree_->set_current(new_node);
}

void Workspace::Remove(Window w) {
    Client* c = GetClient(w);
    if (!c) return;

    TreeNode* node = client_tree_->GetTreeNode(c);
    if (node == client_tree_->root()) return;

    // Get all tiling leaves and find the index of the node we're going to remove.
    vector<TreeNode*> nodes = client_tree_->GetAllLeaves();
    nodes.erase(remove_if(nodes.begin(), nodes.end(), [](TreeNode* n) {
            return n->client()->is_floating(); }), nodes.end());
    ptrdiff_t idx = find(nodes.begin(), nodes.end(), node) - nodes.begin();

    // Remove this node from its parent.
    TreeNode* parent_node = node->parent();
    parent_node->RemoveChild(node);
    delete node;

    // If its parent has no children left, then remove parent from
    // its grandparent (If this parent is not the root).
    if (parent_node != client_tree_->root() && parent_node->children().empty()) {
        TreeNode* grandparent_node = parent_node->parent();
        grandparent_node->RemoveChild(parent_node);
        delete parent_node;
    }

    // Decide which node shall be set as the new current TreeNode.
    // If there are no windows left, set current to nullptr.
    if (nodes.size() - 1 == 0) {
        client_tree_->set_current(nullptr);
        return;
    }
    // If idx is out of bounds, decrement it by one.
    if (idx >= (long) nodes.size() - 1) {
        idx--;
    }
    client_tree_->set_current(nodes[idx]);
}

void Workspace::Move(Window w, Workspace* new_workspace) {
    bool is_floating = Client::mapper_[w]->is_floating();
    Remove(w);
    new_workspace->Add(w, is_floating);
}

void Workspace::Arrange() {
    // If there are no clients to arrange, return at once.
    if (!client_tree_->current()) return;

    // Get display resolution.
    pair<int, int> display_resolution = wm_utils::GetDisplayResolution(dpy_, root_window_);
    int screen_width = display_resolution.first;
    int screen_height = display_resolution.second;
    Tile(client_tree_->root(), 0, 0, screen_width, screen_height);
}

void Workspace::Tile(TreeNode* node, int x, int y, int width, int height) {
    // Retrieve all clients that we should tile.
    vector<TreeNode*> tiling_children;
    for (const auto& child : node->children()) {
        if (!child->client()->is_floating()) {
            tiling_children.push_back(child);
        }
    }

    Direction dir = node->tiling_direction();
    int child_x = x;
    int child_y = y;
    int child_width = (dir == Direction::HORIZONTAL) ? width / tiling_children.size() : width;
    int child_height = (dir == Direction::VERTICAL) ? height / tiling_children.size() : height;

    for (size_t i = 0; i < tiling_children.size(); i++) {
        TreeNode* child = node->children()[i];
        if (node->tiling_direction() == Direction::HORIZONTAL) child_x = x + i * child_width;
        if (node->tiling_direction() == Direction::VERTICAL) child_y = y + i * child_height;

        if (child->IsLeaf()) {
            XMoveResizeWindow(dpy_, child->client()->window(), child_x, child_y, child_width, child_height);
        } else {
            Tile(child, child_x, child_y, child_width, child_height);
        }
    }
}

void Workspace::SetTilingDirection(Direction tiling_direction) {
    client_tree_->current()->set_tiling_direction(tiling_direction);
}


void Workspace::MapAllClients() {
    for (auto leaf : client_tree_->GetAllLeaves()) {
        XMapWindow(dpy_, leaf->client()->window());
    }
}

void Workspace::UnmapAllClients() {
    for (auto leaf : client_tree_->GetAllLeaves()) {
        XUnmapWindow(dpy_, leaf->client()->window());
    }
}

void Workspace::RaiseAllFloatingClients() {
    for (auto c : GetFloatingClients()) {
        XRaiseWindow(dpy_, c->window());
    }
}

void Workspace::SetFocusedClient(Window w) {
    Client* c = Client::mapper_[w];
    if (c) {
        // Raise the window to the top and set input focus to it.
        XRaiseWindow(dpy_, w);
        XSetInputFocus(dpy_, w, RevertToParent, CurrentTime);
        c->SetBorderColor(Config::GetInstance()->focused_color());
    }
}

void Workspace::UnsetFocusedClient() {
    if (!client_tree_->current()) return;

    Client* c = client_tree_->current()->client();
    if (c) {
        c->SetBorderColor(Config::GetInstance()->unfocused_color());
    }
}


Client* Workspace::GetFocusedClient() const {
    if (!client_tree_->current()) return nullptr;
    return client_tree_->current()->client();
}

Client* Workspace::GetClient(Window w) const {
    // We'll get the corresponding client using the lightning fast
    // client mapper which has bigO(1), so we don't have to iterate
    // through the two dimensional clients_ vector!
    Client* c = Client::mapper_[w];
    // But we have to check if it belongs to current workspace!
    return (c && c->workspace() == this) ? c : nullptr;
}

vector<Client*> Workspace::GetFloatingClients() const {
    vector<Client*> floating_clients;

    for (auto leaf : client_tree_->GetAllLeaves()) {
        if (leaf->client()->is_floating()) {
            floating_clients.push_back(leaf->client());
        }
    }
    return floating_clients;
}

vector<Client*> Workspace::GetTilingClients() const {
    vector<Client*> tiling_clients;

    for (auto leaf : client_tree_->GetAllLeaves()) {
        tiling_clients.push_back(leaf->client());
    }
    return tiling_clients;
}


void Workspace::FocusLeft() {
    TreeNode* ptr = client_tree_->current();

    while (ptr) {
        TreeNode* left_sibling = ptr->GetLeftSibling();

        if (ptr->parent()->tiling_direction() == Direction::HORIZONTAL && left_sibling) {
            ptr = left_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().back();
            }
            SetFocusedClient(ptr->client()->window());
            return;
        } else {
            if (ptr->parent() == client_tree_->root()) return;
            ptr = ptr->parent();
        }
    }
}

void Workspace::FocusRight() {
    TreeNode* ptr = client_tree_->current();

    while (ptr) {
        TreeNode* right_sibling = ptr->GetRightSibling();

        if (ptr->parent()->tiling_direction() == Direction::HORIZONTAL && right_sibling) {
            ptr = right_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().front();
            }
            SetFocusedClient(ptr->client()->window());
            return;
        } else {
            if (ptr->parent() == client_tree_->root()) return;
            ptr = ptr->parent();
        }
    }
}

void Workspace::FocusUp() {
    TreeNode* ptr = client_tree_->current();

    while (ptr) {
        TreeNode* left_sibling = ptr->GetLeftSibling();

        if (ptr->parent()->tiling_direction() == Direction::VERTICAL && left_sibling) {
            ptr = left_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().back();
            }
            SetFocusedClient(ptr->client()->window());
            return;
        } else {
            if (ptr->parent() == client_tree_->root()) return;
            ptr = ptr->parent();
        }
    }
}

void Workspace::FocusDown() {
    TreeNode* ptr = client_tree_->current();

    while (ptr) {
        TreeNode* right_sibling = ptr->GetRightSibling();

        if (ptr->parent()->tiling_direction() == Direction::VERTICAL && right_sibling) {
            ptr = right_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().front();
            }
            SetFocusedClient(ptr->client()->window());
            return;
        } else {
            if (ptr->parent() == client_tree_->root()) return;
            ptr = ptr->parent();
        }
    }
}


int Workspace::id() {
    return id_;
}

bool Workspace::is_fullscreen() {
    return is_fullscreen_;
}

void Workspace::set_fullscreen(bool is_fullscreen) {
    is_fullscreen_ = is_fullscreen;
}
