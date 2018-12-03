#include "config.hpp"
#include "util.hpp"
#include <fstream>

using std::string;
using std::vector;

Config::Config(string filename) {
    // If the file starts with ~, convert it to full path first.
    if (filename.at(0) == '~') {
        filename = string(getenv("HOME")) + filename.substr(1, string::npos);
    }

    std::ifstream file(filename);
    string line;

    while (std::getline(file, line)) {
        string_utils::trim(line);

        if (line.at(0) != ';') {
            vector<string> tokens = string_utils::split(line, ' ');

            if (tokens[0] == "set") {

            } else if (tokens[0] == "map") {

            } else if (tokens[0] == "exec") {
                string cmd = string_utils::split(line, ' ', 1)[1];
                autostart_rules_.push_back(cmd);
            } else {

            }

        }
    }
}


/*
string Config::Get(const string& key) const {
}

bool Config::Has(const string& key) const {
}
*/

vector<string>& Config::autostart_rules() {
    return autostart_rules_;
}
