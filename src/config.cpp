#include "config.hpp"
#include "util.hpp"
#include <fstream>
#include <sstream>

using std::hex;
using std::stringstream;
using std::unordered_map;
using std::string;
using std::vector;

Config* Config::instance_;

Config* Config::GetInstance() {
    if (!instance_) {
        instance_ = new Config(CONFIG_FILE);
    }
    return instance_;
}

Config::Config(string filename) { 
    // If the file starts with ~, convert it to full path first.
    if (filename.at(0) == '~') {
        filename = string(getenv("HOME")) + filename.substr(1, string::npos);
    }

    std::ifstream file(filename);
    string line;

    while (std::getline(file, line)) {
        string_utils::trim(line);

        if (!line.empty() && line.at(0) != ';') {
            vector<string> tokens = string_utils::split(line, ' ');

            if (tokens[0] == "set") {
                string key = tokens[1];
                string value = tokens[3];
                global_vars_[key] = value;
            } else if (tokens[0] == "map") {

            } else if (tokens[0] == "exec") {
                string cmd = string_utils::split(line, ' ', 1)[1];
                autostart_rules_.push_back(cmd);
            } else {

            }

        }
    }

    // Load the config with default values first.
    gap_width_ = DEFAULT_GAP_WIDTH;
    border_width_ = DEFAULT_BORDER_WIDTH;
    min_window_width_ = MIN_WINDOW_WIDTH;
    min_window_height_ = MIN_WINDOW_HEIGHT;
    focused_color_ = DEFAULT_FOCUSED_COLOR;
    unfocused_color_ = DEFAULT_UNFOCUSED_COLOR;

    // Override default values from the config file.
    stringstream(global_vars_["gap_width"]) >> gap_width_;
    stringstream(global_vars_["border_width"]) >> border_width_;
    stringstream(global_vars_["focused_color"]) >> hex >> focused_color_;
    stringstream(global_vars_["unfocused_color"]) >> hex >> unfocused_color_;
}


unsigned short Config::gap_width() {
    return gap_width_;
}

unsigned short Config::border_width() {
    return border_width_;
}

unsigned short Config::min_window_width() {
    return min_window_width_;
}

unsigned short Config::min_window_height() {
    return min_window_height_;
}

unsigned long Config::focused_color() {
    return focused_color_;
}

unsigned long Config::unfocused_color() {
    return unfocused_color_;
}

/*
string Config::Get(const string& key) const {
}

bool Config::Has(const string& key) const {
}
*/

unordered_map<string, string>& Config::global_vars() {
    return global_vars_;
}

vector<string>& Config::autostart_rules() {
    return autostart_rules_;
}
