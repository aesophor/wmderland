// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "tree.h"

#include <algorithm>
#include <queue>
#include <stack>

#include "client.h"
#include "snapshot.h"
#include "util.h"

using std::queue;
using std::stack;
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::vector;

namespace wmderland {

unordered_map<Client*, Tree::Node*> Tree::Node::mapper_;

Tree::Tree() : root_node_(std::make_unique<Tree::Node>(nullptr)), current_node_() {
  // NOTE: In wmderland, the root node will always exist in a client tree
  // at any given time.

  // Initialize a Tree::Node with no client associated with it,
  // and set its tiling direction to HORIZONTAL by default.
  root_node_->set_tiling_direction(TilingDirection::HORIZONTAL);
}

Tree::Node* Tree::GetTreeNode(Client* client) const {
  auto it = Tree::Node::mapper_.find(client);
  if (it == Tree::Node::mapper_.end()) {
    return nullptr;
  }
  return it->second;
}

vector<Tree::Node*> Tree::GetLeaves() const {
  return root_node_->GetLeaves();
}

Tree::Node* Tree::root_node() const {
  return root_node_.get();
}

Tree::Node* Tree::current_node() const {
  return current_node_;
}

void Tree::set_current_node(Tree::Node* node) {
  current_node_ = node;
}

void Tree::Deserialize(string data) {
  // Extract current window id at the beginning (before '|'),
  // and then erase it including the '|' character.
  size_t delim_pos = data.find('|');
  string current_window = data.substr(0, delim_pos);
  data.erase(0, delim_pos + 1);

  // Split nodes into a queue.
  // A token can be a leaf or an internal node.
  queue<string> val_queue;
  for (const auto& token : string_utils::Split(data, ',')) {
    val_queue.push(token);
  }

  // Construct root_node_.
  string root_val = val_queue.front();
  val_queue.pop();
  root_val.erase(0, 1);
  root_node_->set_tiling_direction(static_cast<TilingDirection>(std::stoi(root_val)));

  // Push the root node onto the stack and perform iterative DFS.
  stack<Tree::Node*> st;
  st.push(root_node_.get());
  current_node_ = st.top();

  while (!val_queue.empty()) {
    string val = val_queue.front();
    val_queue.pop();

    // Backtrack
    if (val == Snapshot::kBacktrack_) {
      st.pop();
      current_node_ = st.top();
      continue;
    }

    // Deserialize
    unique_ptr<Tree::Node> new_node = std::make_unique<Tree::Node>(nullptr);
    if (val.front() == Snapshot::kLeafPrefix_) {
      val.erase(0, 1);
      unique_ptr<Client> client(Client::mapper_[static_cast<Window>(std::stoul(val))]);
      new_node->set_client(std::move(client));
    } else {  // val.front() == Snapshot::kInternalPrefix_
      val.erase(0, 1);
      new_node->set_tiling_direction(static_cast<TilingDirection>(std::stoi(val)));
    }

    current_node_->AddChild(std::move(new_node));
    current_node_->children().back();
    st.push(new_node.get());
  }

  // Restore current_node_.
  if (current_window == Snapshot::kNone_) {
    current_node_ = nullptr;
  } else {
    Window window = static_cast<Window>(std::stoul(current_window));
    Client* client = Client::mapper_.at(window);
    current_node_ = Tree::Node::mapper_.at(client);
  }
}

string Tree::Serialize() const {
  string data;

  // The current_node_ is serialized and stored at the beginning of data.
  if (current_node_) {
    data += std::to_string(current_node_->client()->window());
  } else {
    data += Snapshot::kNone_;
  }
  data += '|';

  // Serialize the entire tree.
  if (root_node_->leaf()) {
    return data + Snapshot::kInternalPrefix_ +
        std::to_string(static_cast<int>(root_node_->tiling_direction()));
  } else {
    DfsSerializeHelper(root_node_.get(), data);
    // There will be extra ',' + "b,"
    return data.erase(data.rfind("," + Snapshot::kBacktrack_ + ","));
  }
}

void Tree::DfsSerializeHelper(Tree::Node* node, string& data) const {
  if (!node) {
    return;
  }

  // Serialize current node, format:
  // 1. If node is a leaf: w<Window>
  // 2. If node is internal: i<TilingDirection>
  //
  // Exception: when there are no windows at all, the root node will become a
  // leaf, but we don't want this to happen! (It will segfault)
  //
  // Solution: when the root node is a leaf, we don't have to serialize
  // anything.
  if (node->leaf()) {
    data += Snapshot::kLeafPrefix_ + std::to_string(node->client()->window());
  } else {
    data += Snapshot::kInternalPrefix_ +
        std::to_string(static_cast<int>(node->tiling_direction()));
  }
  data += ',';

  for (const auto child : node->children()) {
    DfsSerializeHelper(child, data);
  }

  // Add 'b' which indicates when to backtrack
  // during deserialization.
  data += Snapshot::kBacktrack_ + ',';
}

Tree::Node::Node(unique_ptr<Client> client)
    : client_(std::move(client)), tiling_direction_(TilingDirection::UNSPECIFIED) {
  if (client_) {
    Tree::Node::mapper_[client_.get()] = this;
  }
}

Tree::Node::~Node() {
  Tree::Node::mapper_.erase(client_.get());
}

void Tree::Node::AddChild(unique_ptr<Tree::Node> child) {
  child->set_parent(this);
  children_.push_back(std::move(child));
}

void Tree::Node::RemoveChild(Tree::Node* child) {
  child->set_parent(nullptr);
  children_.erase(
      std::remove_if(children_.begin(), children_.end(),
                     [&](unique_ptr<Tree::Node>& node) { return node.get() == child; }),
      children_.end());
}

void Tree::Node::InsertChildAfter(unique_ptr<Tree::Node> child, Tree::Node* ref) {
  child->set_parent(this);
  ptrdiff_t ref_idx =
      std::find_if(children_.begin(), children_.end(),
                   [&](unique_ptr<Tree::Node>& node) { return node.get() == ref; }) -
      children_.begin();
  children_.insert(children_.begin() + ref_idx + 1, std::move(child));
}

void Tree::Node::Swap(Tree::Node* destination) {
  if (!leaf() || !destination->leaf() || !parent_ || !destination->parent_) {
    return;
  }

  unique_ptr<Tree::Node>& this_ptr = owning_pointer_();
  unique_ptr<Tree::Node>& dest_ptr = destination->owning_pointer_();

  this_ptr.swap(dest_ptr);
  std::swap(parent_, destination->parent_);
}

Tree::Node* Tree::Node::GetLeftSibling() const {
  vector<Tree::Node*> siblings = parent_->children();

  if (this == siblings.front()) {
    return nullptr;
  } else {
    ptrdiff_t this_node_idx =
        std::find(siblings.begin(), siblings.end(), this) - siblings.begin();
    return siblings[this_node_idx - 1];
  }
}

Tree::Node* Tree::Node::GetRightSibling() const {
  vector<Tree::Node*> siblings = parent_->children();

  if (this == siblings.back()) {
    return nullptr;
  } else {
    ptrdiff_t this_node_idx =
        std::find(siblings.begin(), siblings.end(), this) - siblings.begin();
    return siblings[this_node_idx + 1];
  }
}

vector<Tree::Node*> Tree::Node::GetLeaves() {
  vector<Tree::Node*> leaves;
  stack<Tree::Node*> st;
  st.push(this);

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

bool Tree::Node::HasTilingClientsInSubtree() {
  // Get all leaves from this node's subtree,
  // and check if it has any tiling client.
  for (auto leaf : this->GetLeaves()) {
    if (!leaf->client()->is_floating()) {
      return true;
    }
  }
  return false;
}

vector<Tree::Node*> Tree::Node::children() const {
  vector<Tree::Node*> children(children_.size());
  for (size_t i = 0; i < children.size(); i++) {
    children[i] = children_[i].get();
  }
  return children;
}

Tree::Node* Tree::Node::parent() const {
  return parent_;
}

void Tree::Node::set_parent(Tree::Node* parent) {
  parent_ = parent;
}

// Get the client associated with this node.
Client* Tree::Node::client() const {
  return client_.get();
}

void Tree::Node::set_client(unique_ptr<Client> client) {
  Tree::Node::mapper_.erase(client_.get());
  if (client) {
    Tree::Node::mapper_[client.get()] = this;
  }
  client_ = std::move(client);
}

Client* Tree::Node::release_client() {
  return client_.release();
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

unique_ptr<Tree::Node>& Tree::Node::owning_pointer_() const {
  return *std::find_if(parent_->children_.begin(), parent_->children_.end(),
                       [&](unique_ptr<Tree::Node>& node) { return node.get() == this; });
}

}  // namespace wmderland
