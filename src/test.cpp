#include <iostream>
#include <vector>

#include "tiling.hpp"
#include "tree.hpp"

using std::cout;
using std::endl;
using std::vector;
using tiling::Action;

int main(int argc, char* args[]) {
    Tree* window_tree = new Tree();
    vector<TreeNode*> nodes;

    for (int i = 0; i < 16; i++) {
        nodes.push_back(new TreeNode(i));
    }


    window_tree->set_root(nodes[0]);
    nodes[0]->AddChild(nodes[1]);
    nodes[0]->AddChild(nodes[2]);

    nodes[1]->AddChild(nodes[3]);
    nodes[1]->AddChild(nodes[4]);
    nodes[1]->AddChild(nodes[5]);

    nodes[4]->AddChild(nodes[6]);
    nodes[4]->AddChild(nodes[7]);
    nodes[4]->AddChild(nodes[8]);

    nodes[6]->AddChild(nodes[9]);
    nodes[6]->AddChild(nodes[10]);
    nodes[6]->AddChild(nodes[11]);

    nodes[2]->AddChild(nodes[12]);
    nodes[2]->AddChild(nodes[13]);

    nodes[13]->AddChild(nodes[15]);

    cout << *window_tree << endl;


    for (int i = 1; i < 16; i++) {
        delete nodes[i];
    }
    delete window_tree;
}
