#ifndef WMDERLAND_TREE_H_
#define WMDERLAND_TREE_H_

#include "tree_node.h"

#include <memory>
#include <vector>

class Client;

class Tree {
public:
  Tree();
  virtual ~Tree();

  TreeNode* GetTreeNode(Client* client) const;
  std::vector<TreeNode*> GetAllLeaves() const;

  TreeNode* root() const;
  TreeNode* current() const;
  void set_current(TreeNode* current);

private:
  std::unique_ptr<TreeNode> root_;
  TreeNode* current_;
};

#endif
