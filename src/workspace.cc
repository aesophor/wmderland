// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "workspace.h"

#include <algorithm>
#include <stack>

#include "client.h"
#include "util.h"
#include "window_manager.h"

using std::stack;
using std::string;
using std::unique_ptr;
using std::vector;

namespace wmderland {

class WindowManager;

Workspace::Workspace(Display* dpy, /*Window root_window,*/ Config* config, int id)
    : dpy_(dpy),
//      root_window_(root_window),
      config_(config),
      client_tree_(),
      id_(id),
      name_(std::to_string(id)),
      is_fullscreen_() {}

bool Workspace::Has(Window window) const {
  return GetClient(window) != nullptr;
}

void Workspace::Add(Window window, TilingPosition tiling_position) {
  unique_ptr<Client> client = std::make_unique<Client>(dpy_, window, this);
  unique_ptr<Tree::Node> new_node = std::make_unique<Tree::Node>(std::move(client));

  Tree::Node* new_node_raw = new_node.get();

  // If there are no windows at all, then add this new node as the root's child.
  // Otherwise, add this new node as current node's sibling.
  if (!client_tree_.current_node()) {
    client_tree_.root_node()->AddChild(std::move(new_node));
  } else {
    Tree::Node* current_node = client_tree_.current_node();
    current_node->parent()->InsertChildBeside(std::move(new_node), current_node,
                                              tiling_position);
  }

  if (!is_fullscreen_) {
    client_tree_.set_current_node(new_node_raw);
  }
}

void Workspace::Remove(Window window) {
  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  Tree::Node* node = client_tree_.GetTreeNode(c);
  if (!node) {
    return;
  }

  // Get leaves and find the index of the node we're going to remove.
  vector<Tree::Node*> nodes = client_tree_.GetLeaves();
  ptrdiff_t idx = find(nodes.begin(), nodes.end(), node) - nodes.begin();

  // Remove this node from its parent.
  Tree::Node* parent_node = node->parent();
  parent_node->RemoveChild(node);

  // If its parent has no children left, then remove parent from its grandparent
  // (If this parent is not the root).
  while (parent_node->children().empty() && parent_node != client_tree_.root_node()) {
    Tree::Node* grandparent_node = parent_node->parent();
    grandparent_node->RemoveChild(parent_node);
    parent_node = grandparent_node;
  }

  // Decide which node shall be set as the new current Tree::Node. If there are
  // no windows left, set current to nullptr.
  nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());

  if (nodes.empty()) {
    client_tree_.set_current_node(nullptr);
    return;
  }

  // If idx is out of bound, decrement it by one.
  if (idx >= static_cast<ptrdiff_t>(nodes.size())) {
    idx--;
  }
  client_tree_.set_current_node(nodes[idx]);
}

// The tree with redundant internal nodes looks the same as the tree without them though they
// result the different behavior on the window insertion.
// Thus, to prevent users hitting inconsistent behaviors, we normalize the tree after removing
// clients.
void Workspace::Normalize() {
  client_tree_.Normalize();
}

void Workspace::Move(Window window, Workspace* new_workspace) {
  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  bool is_floating = c->is_floating();
  bool has_unmap_req_from_wm = c->has_unmap_req_from_wm();

  // This will delete current Client* c, and erase it from Client::mapper_
  this->Remove(window);

  // This will allocate a new Client* c', and insert it into Client::mapper_
  new_workspace->Add(window);

  // Transfer old client's state to the new client.
  c = new_workspace->GetClient(window);
  c->set_floating(is_floating);
  c->set_fullscreen(false);
  c->set_has_unmap_req_from_wm(has_unmap_req_from_wm);

  client_tree_.Normalize();
}

void Workspace::Move(Window window, Window ref, AreaType area_type,
                     TilingDirection tiling_direction, TilingPosition tiling_position) {
  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  Client* ref_client = GetClient(ref);
  if (!ref_client) {
    return;
  }

  Tree::Node* ref_node = client_tree_.GetTreeNode(ref_client);

  // We handle four cases here:
  // 1. The window was released in the middle area of `ref` window.
  // 2. The window was released on the edge of `ref` window and -
  //     a. - `ref` window is tiled in the same direction as `tiling_direction`.
  //     b. - `ref` window is NOT tiled in the same direction as `tiling_direction` but its
  //          parent window is.
  //     c. - both of `ref` window and its parent are tiled in the different direction from
  //          `tiling_direction`.
  switch (area_type) {
    case AreaType::MID:
      MoveAndSplit(window, ref, tiling_direction, tiling_position, false);
      break;
    case AreaType::EDGE:
      if (ref_node->parent()->tiling_direction() == tiling_direction) {
        MoveAndInsert(window, ref, tiling_position);
      } else if (ref_node->parent()->parent() &&
                 ref_node->parent()->parent()->tiling_direction() == tiling_direction) {
        MoveAndInsert(window, ref, tiling_position, true);
      } else {
        MoveAndSplit(window, ref, tiling_direction, tiling_position, true);
      }
      break;
    default:
      break;
  }

  client_tree_.Normalize();
}

void Workspace::MoveAndSplit(Window window, Window ref, TilingDirection tiling_direction,
                             TilingPosition tiling_position, bool branch_outer) {
  if (window == ref) {
    return;
  }

  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  Client* ref_client = GetClient(ref);
  if (!ref_client) {
    return;
  }

  Tree::Node* ref_node = client_tree_.GetTreeNode(ref_client);
  if (!ref_node) {
    return;
  }

  bool is_floating = c->is_floating();
  bool is_mapped = c->is_mapped();
  bool has_unmap_req_from_wm = c->has_unmap_req_from_wm();

  Remove(window);

  unique_ptr<Tree::Node> new_node = std::make_unique<Tree::Node>(nullptr);
  Tree::Node* new_node_raw = new_node.get();
  Tree::Node* original_parent_node = ref_node->parent();

  // If `branch_outer` is true, wrap the container containing `ref` by new container and put
  // `window` as sibling of the wrapped container.
  // Otherwise, split the area where `ref` is placed to put `ref` and `window` there.
  if (branch_outer) {
    original_parent_node->InsertChildAboveChildren(std::move(new_node));
    client_tree_.set_current_node(new_node_raw);

    new_node_raw->set_tiling_direction(original_parent_node->tiling_direction());
    original_parent_node->set_tiling_direction(tiling_direction);
  } else {
    ref_node->InsertParent(std::move(new_node));
    client_tree_.set_current_node(ref_node);

    new_node_raw->set_tiling_direction(tiling_direction);
  }

  Add(window, tiling_position);

  // Transfer old client's state to the new client.
  c = ref_client->workspace()->GetClient(window);
  c->set_floating(is_floating);
  c->set_fullscreen(false);
  c->set_mapped(is_mapped);
  c->set_has_unmap_req_from_wm(has_unmap_req_from_wm);
}

void Workspace::MoveAndInsert(Window window, Window ref, TilingPosition tiling_position,
                              bool insert_outer) {
  if (window == ref) {
    return;
  }

  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  Client* ref_client = GetClient(ref);
  if (!ref_client) {
    return;
  }

  // If `insert_outer` is true, insert `window` beside the container containing `ref`.
  // Otherwise, insert `window` beside `ref`.
  Tree::Node* ref_node = insert_outer ? client_tree_.GetTreeNode(ref_client)->parent()
                                      : client_tree_.GetTreeNode(ref_client);
  if (!ref_node) {
    return;
  }

  bool is_floating = c->is_floating();
  bool is_mapped = c->is_mapped();
  bool has_unmap_req_from_wm = c->has_unmap_req_from_wm();

  Remove(window);

  client_tree_.set_current_node(ref_node);
  Add(window, tiling_position);

  // Transfer old client's state to the new client.
  c = ref_client->workspace()->GetClient(window);
  c->set_floating(is_floating);
  c->set_fullscreen(false);
  c->set_mapped(is_mapped);
  c->set_has_unmap_req_from_wm(has_unmap_req_from_wm);
}

void Workspace::Swap(Window window0, Window window1) {
  Client* c0 = GetClient(window0);
  if (!c0) {
    return;
  }

  Tree::Node* node0 = client_tree_.GetTreeNode(c0);
  if (!node0) {
    return;
  }

  Client* c1 = GetClient(window1);
  if (!c1) {
    return;
  }

  Tree::Node* node1 = client_tree_.GetTreeNode(c1);
  if (!node1) {
    return;
  }

  // Swap the states of the clients whether they are floating or not.
  bool is_floating0 = c0->is_floating();
  c0->set_floating(c1->is_floating());
  c1->set_floating(is_floating0);

  node0->Swap(node1);
}

void Workspace::Tile(const Client::Area& tiling_area) const {
  // If there are no clients in this workspace or all clients are floating,
  // return at once.
  if (!client_tree_.current_node() || GetTilingClients().empty()) {
    return;
  }

  int border_width = config_->border_width();
  int gap_width = config_->gap_width();

  int x = tiling_area.x + gap_width / 2;
  int y = tiling_area.y + gap_width / 2;
  int w = tiling_area.w - gap_width;
  int h = tiling_area.h - gap_width;

  DfsTileHelper(client_tree_.root_node(), x, y, w, h, border_width, gap_width);
}

void Workspace::DfsTileHelper(Tree::Node* node, int x, int y, int w, int h, int border_width,
                              int gap_width) const {
  vector<Tree::Node*> children = node->children();

  // We don't care about two kinds of `Tree::Node`s
  // 1. an internal node with no tiling clients in its subtree
  // 2. a leaf but it has a floating client
  // Tree::Node::HasTilingClientsInSubtree() can check 1 and 2 at the same time.
  children.erase(std::remove_if(children.begin(), children.end(),
                                [](Tree::Node* n) { return !n->HasTilingClientsInSubtree(); }),
                 children.end());

  if (children.empty()) {
    return;
  }

  double total_ratio = 0.;
  for (const auto& child : children) {
    total_ratio += child->ratio();
  }

  // Calculate each child's x, y, width and height based on node's tiling
  // direction.
  TilingDirection dir = node->tiling_direction();
  int child_x = x;
  int child_y = y;

  for (size_t i = 0; i < children.size(); i++) {
    Tree::Node* child = children[i];
    int child_width =
        (dir == TilingDirection::HORIZONTAL) ? w * child->ratio() / total_ratio : w;
    int child_height =
        (dir == TilingDirection::VERTICAL) ? h * child->ratio() / total_ratio : h;

    if (child->leaf()) {
      int new_x = child_x + gap_width / 2;
      int new_y = child_y + gap_width / 2;
      int new_width = child_width - border_width * 2 - gap_width;
      int new_height = child_height - border_width * 2 - gap_width;
      child->client()->MoveResize(new_x, new_y, new_width, new_height);
    } else {
      DfsTileHelper(child, child_x, child_y, child_width, child_height, border_width,
                    gap_width);
    }

    if (node->tiling_direction() == TilingDirection::HORIZONTAL) child_x += child_width;
    if (node->tiling_direction() == TilingDirection::VERTICAL) child_y += child_height;
  }
}

void Workspace::SetTilingDirection(TilingDirection tiling_direction) {
  if (!client_tree_.current_node()) {
    client_tree_.root_node()->set_tiling_direction(tiling_direction);
    return;
  }

  Tree::Node* current_node = client_tree_.current_node();

  // If current node has no siblings, we can simply set the new
  // tiling direction on its parent and return.
  if (current_node->parent()->children().size() == 1) {
    current_node->parent()->set_tiling_direction(tiling_direction);
    return;
  }

  unique_ptr<Client> client(current_node->release_client());
  unique_ptr<Tree::Node> new_node = std::make_unique<Tree::Node>(std::move(client));

  // If the user has specified a tiling direction on current node, then
  // 1. Let current node become an internal node.
  // 2. Add the original current node as this internal node's child.
  // 3. Set the first child of this internal node as the new current node.
  current_node->set_tiling_direction(tiling_direction);
  current_node->AddChild(std::move(new_node));
  client_tree_.set_current_node(current_node->children().front());
}

void Workspace::MapAllClients() const {
  for (const auto c : GetClients()) {
    c->Map();
  }
}

void Workspace::UnmapAllClients(Window except_window) const {
  for (const auto c : GetClients()) {
    if (c->window() != except_window) {
      c->Unmap();
    }
  }
}

void Workspace::RaiseAllFloatingClients() const {
  for (const auto c : GetFloatingClients()) {
    c->Raise();
  }
}

void Workspace::SetFocusedClient(Window window) {
  Client* c = GetClient(window);
  if (!c) {
    return;
  }

  // Raise the window to the top and set input focus to it.
  c->Raise();
  c->SetInputFocus();
  c->SetBorderColor(config_->focused_color());
  client_tree_.set_current_node(client_tree_.GetTreeNode(c));
}

void Workspace::UnsetFocusedClient() const {
  if (!client_tree_.current_node()) {
    return;
  }

  Client* c = client_tree_.current_node()->client();
  if (c) {
    c->SetBorderColor(config_->unfocused_color());
  }
}

void Workspace::DisableFocusFollowsMouse() const {
  if (!config_->focus_follows_mouse()) {
    return;
  }

  for (const auto c : GetClients()) {
    c->SelectInput(None);
  }
}

void Workspace::EnableFocusFollowsMouse() const {
  if (!config_->focus_follows_mouse()) {
    return;
  }

  for (const auto c : GetClients()) {
    c->SelectInput(EnterWindowMask);
  }
}

void Workspace::ResizeTiled(Action::Type resize_action_type, int deltaPercentage) {
  if (!client_tree_.current_node() || !client_tree_.current_node()->parent() ||
      this->is_fullscreen()) {
    return;
  }

  Tree::Node* node = client_tree_.current_node();
  TilingDirection target_direction;
  switch (resize_action_type) {
    case Action::Type::RESIZE_WIDTH:
      target_direction = TilingDirection::HORIZONTAL;
      break;
    case Action::Type::RESIZE_HEIGHT:
      target_direction = TilingDirection::VERTICAL;
      break;
    default:
      return;
  }
  while (node->parent() && node->parent()->tiling_direction() != target_direction) {
    node = node->parent();
  }

  node->Resize(deltaPercentage * 0.01);
}

void Workspace::ResizeTiledToRatio(int percentage) {
  client_tree_.current_node()->ResizeToRatio(percentage * 0.01);
}

void Workspace::ResizeDistributeRatios() {
  if (!client_tree_.current_node()->parent()) {
    return;
  }

  client_tree_.current_node()->parent()->DistributeChildrenRatios();
}

void Workspace::Navigate(Action::Type focus_action_type) {
  // Do not let user navigate between windows if
  // 1. there's no currently focused client
  // 2. current workspace is in fullscreen mode (there's a fullscreen window)
  if (!client_tree_.current_node() || this->is_fullscreen()) {
    return;
  }

  TilingDirection target_direction = TilingDirection::HORIZONTAL;
  bool find_leftward = true;

  switch (focus_action_type) {
    case Action::Type::NAVIGATE_LEFT:
      target_direction = TilingDirection::HORIZONTAL;
      find_leftward = true;
      break;
    case Action::Type::NAVIGATE_RIGHT:
      target_direction = TilingDirection::HORIZONTAL;
      find_leftward = false;
      break;
    case Action::Type::NAVIGATE_UP:
      target_direction = TilingDirection::VERTICAL;
      find_leftward = true;
      break;
    case Action::Type::NAVIGATE_DOWN:
      target_direction = TilingDirection::VERTICAL;
      find_leftward = false;
      break;
    default:
      return;
  }

  for (Tree::Node* node = client_tree_.current_node(); node; node = node->parent()) {
    Tree::Node* sibling = (find_leftward) ? node->GetLeftSibling() : node->GetRightSibling();

    if (node->parent()->tiling_direction() == target_direction && sibling) {
      node = sibling;
      while (!node->leaf()) {
        node = (find_leftward) ? node->children().back() : node->children().front();
      }
      UnsetFocusedClient();
      SetFocusedClient(node->client()->window());
      WindowManager::GetInstance()->ArrangeWindows();
      return;
    } else if (node->parent() == client_tree_.root_node()) {
      return;
    }
  }
}

Client* Workspace::GetFocusedClient() const {
  if (!client_tree_.current_node()) {
    return nullptr;
  }
  return client_tree_.current_node()->client();
}

Client* Workspace::GetClient(Window window) const {
  // We'll get the corresponding client using client mapper
  // whose time complexity is O(1).
  auto it = Client::mapper_.find(window);
  if (it == Client::mapper_.end()) {
    return nullptr;
  }

  // But we have to check if it belongs to current workspace!
  Client* c = it->second;
  return (c->workspace() == this) ? c : nullptr;
}

vector<Client*> Workspace::GetClients() const {
  vector<Client*> clients;

  for (auto leaf : client_tree_.GetLeaves()) {
    if (leaf != client_tree_.root_node()) {
      clients.push_back(leaf->client());
    }
  }
  return clients;
}

vector<Client*> Workspace::GetFloatingClients() const {
  vector<Client*> clients = GetClients();
  clients.erase(std::remove_if(clients.begin(), clients.end(),
                               [](Client* c) { return !c->is_floating(); }),
                clients.end());
  return clients;
}

vector<Client*> Workspace::GetTilingClients() const {
  vector<Client*> clients = GetClients();
  clients.erase(std::remove_if(clients.begin(), clients.end(),
                               [](Client* c) { return c->is_floating(); }),
                clients.end());
  return clients;
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

void Workspace::set_name(const string& name) {
  name_ = name;
}

void Workspace::set_fullscreen(bool fullscreen) {
  is_fullscreen_ = fullscreen;
}

string Workspace::Serialize() const {
  return client_tree_.Serialize();
}

void Workspace::Deserialize(string data) {
  client_tree_.Deserialize(data);
}

}  // namespace wmderland
