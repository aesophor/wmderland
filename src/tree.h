// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef WMDERLAND_TREE_H_
#define WMDERLAND_TREE_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "tree_node.h"

namespace wmderland {

class Client;

class Tree {
 public:
  Tree();
  virtual ~Tree() = default;

  TreeNode* GetTreeNode(Client* client) const;
  std::vector<TreeNode*> GetAllLeaves() const;

  TreeNode* root() const;
  TreeNode* current() const;
  void set_current(TreeNode* current);

 private:
  std::unique_ptr<TreeNode> root_;
  TreeNode* current_;
};

} // namespace wmderland

#endif // WMDERLAND_TREE_H_
