#include "action.hpp"
#include "config.hpp"
#include "util.hpp"
#include <glog/logging.h>
#include <fstream>
#include <sstream>

using std::hex;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;
using std::unordered_map;

Config* Config::instance_;

Config* Config::GetInstance() {
    if (!instance_) {
        instance_ = new Config(CONFIG_FILE);
    }
    return instance_;
}

Config::Config(string filename) {
    // Load the default global values.
    gap_width_ = DEFAULT_GAP_WIDTH;
    border_width_ = DEFAULT_BORDER_WIDTH;
    min_window_width_ = MIN_WINDOW_WIDTH;
    min_window_height_ = MIN_WINDOW_HEIGHT;
    focused_color_ = DEFAULT_FOCUSED_COLOR;
    unfocused_color_ = DEFAULT_UNFOCUSED_COLOR;

    // Load the default keybinds.
    SetKeybindActions(DEFAULT_TILE_H_KEY, "tile_horizontally");
    SetKeybindActions(DEFAULT_TILE_V_KEY, "tile_vertically");
    SetKeybindActions(DEFAULT_FOCUS_LEFT_KEY, "focus_left");
    SetKeybindActions(DEFAULT_FOCUS_RIGHT_KEY, "focus_right");
    SetKeybindActions(DEFAULT_FOCUS_DOWN_KEY, "focus_down");
    SetKeybindActions(DEFAULT_FOCUS_UP_KEY, "focus_up");
    SetKeybindActions(DEFAULT_TOGGLE_FLOATING_KEY, "toggle_floating");
    SetKeybindActions(DEFAULT_TOGGLE_FULLSCREEN_KEY, "toggle_fullscreen");
    SetKeybindActions(DEFAULT_KILL_KEY, "kill");
    SetKeybindActions(DEFAULT_EXIT_KEY, "exit");
    
    // If the file starts with ~, convert it to full path first.
    filename = sys_utils::ToAbsPath(filename);

    std::ifstream file(filename);
    string line;

    while (std::getline(file, line)) {
        string_utils::Strip(line);

        if (!line.empty() && line.at(0) != ';') {
            vector<string> tokens = string_utils::Split(ReplaceSymbols(line), ' ');
            ConfigKeyword keyword = Config::StrToConfigKeyword(tokens[0]);

            switch (keyword) {
                case ConfigKeyword::SET: {
                    string key = tokens[1];
                    string value = tokens[3];
                    if (string_utils::StartsWith(key, VARIABLE_PREFIX)) {
                        symtab_[key] = value;
                    } else {
                        global_vars_[key] = value;
                    }
                    break;
                } case ConfigKeyword::ASSIGN: {
                    string wm_class_name = tokens[1];
                    string workspace_id = tokens[3];
                    short workspace_id_short;
                    stringstream(workspace_id) >> workspace_id_short;
                    spawn_rules_[wm_class_name] = workspace_id_short;
                    break;
                } case ConfigKeyword::FLOATING: {
                    string wm_class_name = tokens[1];
                    string should_float = tokens[2];
                    bool should_float_bool;
                    stringstream(should_float) >> std::boolalpha >> should_float_bool;
                    float_rules_[wm_class_name] = should_float_bool;
                    break;
                } case ConfigKeyword::BINDSYM: {
                    string modifier_and_key = tokens[1];
                    string action_series_str = string_utils::Split(line, ' ', 2)[2];
                    SetKeybindActions(modifier_and_key, action_series_str);
                    break;
                } case ConfigKeyword::EXEC: {
                    string cmd = string_utils::Split(line, ' ', 1)[1];
                    autostart_rules_.push_back(cmd);
                    break;
                } default: {
                    LOG(INFO) << "Unrecognized symbol: " << tokens[0] << ". Ignoring...";
                    break;
                }
            }
        }
    }

    file.close();

    // Override default global values with the values specified in config.
    stringstream(global_vars_["gap_width"]) >> gap_width_;
    stringstream(global_vars_["border_width"]) >> border_width_;
    stringstream(global_vars_["min_window_width"]) >> min_window_width_;
    stringstream(global_vars_["min_window_height"]) >> min_window_height_;
    stringstream(global_vars_["focused_color"]) >> hex >> focused_color_;
    stringstream(global_vars_["unfocused_color"]) >> hex >> unfocused_color_;
}

Config::~Config() {}


const vector<Action>& Config::GetKeybindActions(const string& modifier, const string& key) const {
    return keybind_rules_.at(modifier + '+' + key);
}

void Config::SetKeybindActions(const string& modifier_and_key, const string& action_series_str) {
    keybind_rules_[modifier_and_key].clear();

    for (auto& action_str : string_utils::Split(action_series_str, ';')) {
        string_utils::Strip(action_str);
        keybind_rules_[modifier_and_key].push_back(Action(action_str));
    }
}

ConfigKeyword Config::StrToConfigKeyword(const std::string& s) {
    if (s == "set") {
        return ConfigKeyword::SET;
    } else if (s == "assign") {
        return ConfigKeyword::ASSIGN;
    } else if (s == "floating") {
        return ConfigKeyword::FLOATING;
    } else if (s == "bindsym") {
        return ConfigKeyword::BINDSYM;
    } else if (s == "exec") {
        return ConfigKeyword::EXEC;
    } else {
        return ConfigKeyword::UNDEFINED;
    }
}

const string& Config::ReplaceSymbols(string& s) {
    for (auto symtab_record : symtab_) {
        string_utils::Replace(s, symtab_record.first, symtab_record.second);
    }
    return s;
}


unsigned short Config::gap_width() const {
    return gap_width_;
}

unsigned short Config::border_width() const {
    return border_width_;
}

unsigned short Config::min_window_width() const {
    return min_window_width_;
}

unsigned short Config::min_window_height() const {
    return min_window_height_;
}

unsigned long Config::focused_color() const {
    return focused_color_;
}

unsigned long Config::unfocused_color() const {
    return unfocused_color_;
}


const unordered_map<string, string>& Config::global_vars() const {
    return global_vars_;
}

const unordered_map<string, short>& Config::spawn_rules() const {
    return spawn_rules_;
}

const unordered_map<string, bool>& Config::float_rules() const {
    return float_rules_;
}

const unordered_map<string, vector<Action>>& Config::keybind_rules() const {
    return keybind_rules_;
}

const vector<string>& Config::autostart_rules() const {
    return autostart_rules_;
}
