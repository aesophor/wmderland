// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef WMDERLAND_TREE_NODE_H_
#define WMDERLAND_TREE_NODE_H_

#include <vector>
#include <unordered_map>

#include "util.h"

namespace wmderland {

class Client;

class TreeNode {
 public:
  TreeNode(Client* client);
  virtual ~TreeNode();

  // The lightning fast mapper which maps Client* to TreeNode* in O(1)
  static std::unordered_map<Client*, TreeNode*> mapper_;

  bool IsLeaf() const;
  void AddChild(TreeNode* child);
  void RemoveChild(TreeNode* child);
  void InsertChildAfter(TreeNode* child, TreeNode* ref);

  TreeNode* GetLeftSibling() const;
  TreeNode* GetRightSibling() const;

  const std::vector<TreeNode*>& children() const;
  TreeNode* parent() const;
  void set_parent(TreeNode* parent);

  Client* client() const;
  void set_client(Client* client);

  tiling::Direction tiling_direction() const;
  void set_tiling_direction(tiling::Direction tiling_direction);

 private:
  std::vector<TreeNode*> children_;
  TreeNode* parent_;

  Client* client_;
  tiling::Direction tiling_direction_;
};

} // namespace wmderland

#endif // WMDERLAND_TREE_NODE_H_
