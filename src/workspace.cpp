#include "wm.hpp"
#include "workspace.hpp"
#include "util.hpp"
#include <algorithm>
#include <stack>

using std::pair;
using std::stack;
using std::vector;
using std::remove;
using std::remove_if;
using std::shared_ptr;
using tiling::Direction;

class WindowManager;

Workspace::Workspace(Display* dpy, Window root_window, Config* config, int id)
    : dpy_(dpy),
      root_window_(root_window),
      config_(config),
      client_tree_(new Tree()),
      id_(id),
      is_fullscreen_(false) {}

Workspace::~Workspace() {}


bool Workspace::Has(Window w) const {
    return GetClient(w) != nullptr;
}

void Workspace::Add(Window w, bool is_floating) const {
    Client* c = new Client(dpy_, w, (Workspace*) this);
    c->set_floating(is_floating);

    TreeNode* new_node = new TreeNode(c);

    if (!client_tree_->current()) {
        // If there are no windows at all, add the node as the root's child.
        client_tree_->root()->AddChild(new_node);
    } else {
        // If the user has not specified any tiling direction on current node, 
        // then add the new node as its brother.
        TreeNode* current_node = client_tree_->current();
        current_node->parent()->InsertChildAfter(new_node, current_node);
    }

    client_tree_->set_current(new_node);
}

void Workspace::Remove(Window w) const {
    Client* c = GetClient(w);
    if (!c) return;

    TreeNode* node = client_tree_->GetTreeNode(c);
    if (!node) return;

    // Get all tiling leaves and find the index of the node we're going to remove.
    vector<TreeNode*> nodes = client_tree_->GetAllLeaves();
    nodes.erase(remove_if(nodes.begin(), nodes.end(), [](TreeNode* n) {
            return n->client()->is_floating(); }), nodes.end());
    ptrdiff_t idx = find(nodes.begin(), nodes.end(), node) - nodes.begin();

    // Remove this node from its parent.
    TreeNode* parent_node = node->parent();
    parent_node->RemoveChild(node);
    delete node;
    delete c;

    // If its parent has no children left, then remove parent from its grandparent 
    // (If this parent is not the root).
    while (parent_node != client_tree_->root() && parent_node->children().empty()) {
        TreeNode* grandparent_node = parent_node->parent();
        grandparent_node->RemoveChild(parent_node);
        delete parent_node;
        parent_node = grandparent_node;
    }

    // Decide which node shall be set as the new current TreeNode. If there are no
    // windows left, set current to nullptr.
    nodes.erase(remove(nodes.begin(), nodes.end(), node), nodes.end());

    if (nodes.empty()) {
        client_tree_->set_current(nullptr);
        return;
    }
    // If idx is out of bounds, decrement it by one.
    if (idx > (long) nodes.size() - 1) {
        idx--;
    }
    client_tree_->set_current(nodes[idx]);
}

void Workspace::Move(Window w, Workspace* new_workspace) const {
    Client* c = GetClient(w);

    if (c) {
        bool is_floating = c->is_floating();
        Remove(w);
        new_workspace->Add(w, is_floating);
    }
}

void Workspace::Arrange(const Area& tiling_area) const {
    // If there are no clients in this workspace or all clients are floating, return at once.
    if (!client_tree_->current() || GetTilingClients().empty()) return;

    int border_width = config_->border_width();
    int gap_width = config_->gap_width();

    int x = tiling_area.x + gap_width / 2;
    int y = tiling_area.y + gap_width / 2;
    int width = tiling_area.width - gap_width;
    int height = tiling_area.height - gap_width;

    Tile(client_tree_->root(), x, y, width, height, border_width, gap_width);
}

void Workspace::Tile(TreeNode* node, int x, int y, int width, int height, int border_width, int gap_width) const {
    // Retrieve all clients that we should tile.
    vector<TreeNode*> tiling_children;
    for (const auto& child : node->children()) {
        if (child->client() && child->client()->is_floating()) {
            continue;
        } else {
            tiling_children.push_back(child);
        }
    }

    Direction dir = node->tiling_direction();
    int child_x = x;
    int child_y = y;
    int child_width = (dir == Direction::HORIZONTAL) ? width / tiling_children.size() : width;
    int child_height = (dir == Direction::VERTICAL) ? height / tiling_children.size() : height;
    
    for (size_t i = 0; i < tiling_children.size(); i++) {
        TreeNode* child = tiling_children[i];
        if (node->tiling_direction() == Direction::HORIZONTAL) child_x = x + child_width * i;
        if (node->tiling_direction() == Direction::VERTICAL) child_y = y + child_height * i;

        if (child->IsLeaf()) {
            int new_x = child_x + gap_width / 2;
            int new_y = child_y + gap_width / 2;
            int new_width = child_width - border_width * 2 - gap_width;
            int new_height = child_height - border_width * 2 - gap_width;
            XMoveResizeWindow(dpy_, child->client()->window(), new_x, new_y, new_width, new_height);
        } else {
            Tile(child, child_x, child_y, child_width, child_height, border_width, gap_width);
        }
    }
}

void Workspace::SetTilingDirection(Direction tiling_direction) const {
    if (!client_tree_->current()) {
        client_tree_->root()->set_tiling_direction(tiling_direction);
    } else if (client_tree_->current()->parent()->children().size() > 0){
        // If the user has specified a tiling direction on current node, 
        // then set current node as an internal node, add the original
        // current node as this internal node's child and add the new node
        // as this internal node's another child.
        TreeNode* current_node = client_tree_->current();
        current_node->set_tiling_direction(tiling_direction);
        current_node->AddChild(new TreeNode(current_node->client()));
        current_node->set_client(nullptr);
        client_tree_->set_current(current_node->children()[0]);
    }
}


void Workspace::MapAllClients() const {
    for (auto leaf : client_tree_->GetAllLeaves()) {
        if (leaf != client_tree_->root()) {
            leaf->client()->Map();
        }
    }
}

void Workspace::UnmapAllClients() const {
    for (auto leaf : client_tree_->GetAllLeaves()) {
        if (leaf != client_tree_->root()) {
            leaf->client()->Unmap();
        }
    }
}

void Workspace::RaiseAllFloatingClients() const {
    for (auto c : GetFloatingClients()) {
        c->Raise();
    }
}

void Workspace::SetFocusedClient(Window w) const {
    Client* c = GetClient(w);
    if (c) {
        // Raise the window to the top and set input focus to it.
        c->Raise();
        c->SetInputFocus();
        c->SetBorderColor(config_->focused_color());
    }
}

void Workspace::UnsetFocusedClient() const {
    if (!client_tree_->current()) return;

    Client* c = client_tree_->current()->client();
    if (c) {
        c->SetBorderColor(config_->unfocused_color());
    }
}


Client* Workspace::GetFocusedClient() const {
    if (!client_tree_->current()) return nullptr;
    return client_tree_->current()->client();
}

Client* Workspace::GetClient(Window w) const {
    // We'll get the corresponding client using the lightning fast
    // client mapper which has bigO(1).
    Client* c = Client::mapper_[w];
    // But we have to check if it belongs to current workspace!
    return (c && c->workspace() == this) ? c : nullptr;
}

vector<Client*> Workspace::GetFloatingClients() const {
    vector<Client*> floating_clients;

    for (auto leaf : client_tree_->GetAllLeaves()) {
        if (leaf != client_tree_->root() && leaf->client()->is_floating()) {
            floating_clients.push_back(leaf->client());
        }
    }
    return floating_clients;
}

vector<Client*> Workspace::GetTilingClients() const {
    vector<Client*> tiling_clients;

    for (auto leaf : client_tree_->GetAllLeaves()) {
        if (leaf != client_tree_->root() && !leaf->client()->is_floating()) {
            tiling_clients.push_back(leaf->client());
        }
    }
    return tiling_clients;
}


void Workspace::FocusLeft() const {
    for (TreeNode* ptr = client_tree_->current(); ptr; ptr = ptr->parent()) {
        TreeNode* left_sibling = ptr->GetLeftSibling();

        if (ptr->parent()->tiling_direction() == Direction::HORIZONTAL && left_sibling) {
            ptr = left_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().back();
            }
            UnsetFocusedClient();
            SetFocusedClient(ptr->client()->window());
            client_tree_->set_current(ptr);
            return;
        } else if (ptr->parent() == client_tree_->root()) {
            return;
        }
    }
}

void Workspace::FocusRight() const {
    for (TreeNode* ptr = client_tree_->current(); ptr; ptr = ptr->parent()) {
        TreeNode* right_sibling = ptr->GetRightSibling();

        if (ptr->parent()->tiling_direction() == Direction::HORIZONTAL && right_sibling) {
            ptr = right_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().front();
            }
            UnsetFocusedClient();
            SetFocusedClient(ptr->client()->window());
            client_tree_->set_current(ptr);
            return;
        } else if (ptr->parent() == client_tree_->root()) {
            return;
        }
    }
}

void Workspace::FocusUp() const {
    for (TreeNode* ptr = client_tree_->current(); ptr; ptr = ptr->parent()) {
        TreeNode* left_sibling = ptr->GetLeftSibling();

        if (ptr->parent()->tiling_direction() == Direction::VERTICAL && left_sibling) {
            ptr = left_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().back();
            }
            UnsetFocusedClient();
            SetFocusedClient(ptr->client()->window());
            client_tree_->set_current(ptr);
            return;
        } else if (ptr->parent() == client_tree_->root()) {
            return;
        }
    }
}

void Workspace::FocusDown() const {
    for (TreeNode* ptr = client_tree_->current(); ptr; ptr = ptr->parent()) {
        TreeNode* right_sibling = ptr->GetRightSibling();

        if (ptr->parent()->tiling_direction() == Direction::VERTICAL && right_sibling) {
            ptr = right_sibling;
            while (!ptr->IsLeaf()) {
                ptr = ptr->children().front();
            }
            UnsetFocusedClient();
            SetFocusedClient(ptr->client()->window());
            client_tree_->set_current(ptr);
            return;
        } else if (ptr->parent() == client_tree_->root()) {
            return;
        }
    }
}


Config* Workspace::config() const {
    return config_;
}

int Workspace::id() const {
    return id_;
}

const char* Workspace::name() const {
    return name_.c_str();
}

bool Workspace::is_fullscreen() const {
    return is_fullscreen_;
}

void Workspace::set_fullscreen(bool is_fullscreen) {
    is_fullscreen_ = is_fullscreen;
}
