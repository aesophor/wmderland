// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "tree.h"

#include <algorithm>
#include <stack>
#include <queue>

#include "client.h"
#include "util.h"

using std::string;
using std::stack;
using std::queue;
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
  DfsCleanUpHelper(root_node_);
}

void Tree::DfsCleanUpHelper(Tree::Node* node) const {
  if (!node) {
    return;
  }

  for (const auto child : node->children()) {
    DfsCleanUpHelper(child);
  }
  delete node;
}


Tree::Node* Tree::GetTreeNode(Client* client) const {
  auto it = Tree::Node::mapper_.find(client);
  if (it == Tree::Node::mapper_.end()) {
    return nullptr;
  }
  return it->second;
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
  root_node_ = new Tree::Node(nullptr);
  root_node_->set_tiling_direction(static_cast<TilingDirection>(std::stoi(root_val)));
 

  // Push the root node onto the stack and perform iterative DFS.
  stack<Tree::Node*> st;
  st.push(root_node_);
  current_node_ = st.top();

  while (!val_queue.empty()) {
    string val = val_queue.front();
    val_queue.pop();

    // Backtrack
    if (val == "b") {
      st.pop();
      current_node_ = st.top();
      continue;
    }

    // Deserialize
    Tree::Node* new_node = new Tree::Node(nullptr);
    if (val.front() == 'w') {
      val.erase(0, 1);
      new_node->set_client(Client::mapper_[static_cast<Window>(std::stoul(val))]);
    } else { // val.front() == 'i'
      val.erase(0, 1);
      new_node->set_tiling_direction(static_cast<TilingDirection>(std::stoi(val)));
    }

    current_node_->AddChild(new_node);
    current_node_ = new_node;
    st.push(new_node);
  }


  // Restore current_node_.
  if (current_window == "none") {
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
    data += "none";
  }
  data += '|';

  // Serialize the entire tree.
  if (root_node_->leaf()) {
    return data + "i" + std::to_string(static_cast<int>(root_node_->tiling_direction()));
  } else {
    DfsSerializeHelper(root_node_, data);
    return data.erase(data.rfind(",b,")); // there will be extra ',' + "b,"
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
  // Exception: when there are no windows at all, the root node will become a leaf,
  // but we don't want this to happen! (It will segfault)
  //
  // Solution: when the root node is a leaf, we don't have to serialize anything.
  if (node->leaf()) {
    data += 'w' + std::to_string(node->client()->window());
  } else {
    data += 'i' + std::to_string(static_cast<int>(node->tiling_direction()));
  }
  data += ',';

  for (const auto child : node->children()) {
    DfsSerializeHelper(child, data);
  }

  // Add 'b' which indicates when to backtrack
  // during deserialization.
  data += "b,";
}



Tree::Node::Node(Client* client)
    : client_(client),
      tiling_direction_(TilingDirection::UNSPECIFIED) {
  if (client) {
    Tree::Node::mapper_[client] = this;
  }
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
  Tree::Node::mapper_.erase(client_);
  Tree::Node::mapper_[client] = this;
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
