// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef WMDERLAND_TREE_H_
#define WMDERLAND_TREE_H_

#include <memory>
#include <vector>
#include <unordered_map>

#include "util.h"

namespace wmderland {

class Client;

class Tree {
 public:
  Tree();
  virtual ~Tree() = default;
 
  class Node {
   public:
    Node(Client* client);
    virtual ~Node();

    void AddChild(Tree::Node* child);
    void RemoveChild(Tree::Node* child);
    void InsertChildAfter(Tree::Node* child, Tree::Node* ref);
   
    Tree::Node* GetLeftSibling() const;
    Tree::Node* GetRightSibling() const;

    const std::vector<Tree::Node*>& children() const;
    Tree::Node* parent() const;
    Client* client() const;
    tiling::Direction tiling_direction() const;
    bool leaf() const;

    void set_parent(Tree::Node* parent);
    void set_client(Client* client);
    void set_tiling_direction(tiling::Direction tiling_direction);

    static std::unordered_map<Client*, Tree::Node*> mapper_;

   private:
    std::vector<Tree::Node*> children_;
    Tree::Node* parent_;

    Client* client_;
    tiling::Direction tiling_direction_;
  };


  Tree::Node* GetTreeNode(Client* client) const;
  std::vector<Tree::Node*> GetAllLeaves() const;

  Tree::Node* root_node() const;
  Tree::Node* current_node() const;
  void set_current_node(Tree::Node* node);

 private:
  std::unique_ptr<Tree::Node> root_node_;
  Tree::Node* current_node_;
};

} // namespace wmderland

#endif // WMDERLAND_TREE_H_
