#include "util.hpp"
#include "config.hpp"
#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;

int main() {
    Config* c = Config::GetInstance();

    for (auto s : c->spawn_rules()) {
        cout << s.first << "->" << s.second << endl;
    }
}
