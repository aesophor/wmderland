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

Config::Config(string filename) {
    unordered_map<string, string> global_vars;

    // Load the default global values.
    gap_width_ = DEFAULT_GAP_WIDTH;
    border_width_ = DEFAULT_BORDER_WIDTH;
    min_window_width_ = MIN_WINDOW_WIDTH;
    min_window_height_ = MIN_WINDOW_HEIGHT;
    focused_color_ = DEFAULT_FOCUSED_COLOR;
    unfocused_color_ = DEFAULT_UNFOCUSED_COLOR;

    // Load the default keybinds.
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
                        global_vars[key] = value;
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
                    string should_float_str = tokens[2];
                    bool should_float;
                    stringstream(should_float_str) >> std::boolalpha >> should_float;
                    float_rules_[wm_class_name] = should_float;
                    break;
                } case ConfigKeyword::PROHIBIT: {
                    string wm_class_name = tokens[1];
                    string should_prohibit_str = tokens[2];
                    bool should_prohibit;
                    stringstream(should_prohibit_str) >> std::boolalpha >> should_prohibit;
                    prohibit_rules_[wm_class_name] = should_prohibit;
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
                    LOG(INFO) << "Ignored unrecognized symbol in config: " << tokens[0];
                    break;
                }
            }
        }
    }

    file.close();

    // Override default global values with the values specified in config.
    stringstream(global_vars["gap_width"]) >> gap_width_;
    stringstream(global_vars["border_width"]) >> border_width_;
    stringstream(global_vars["min_window_width"]) >> min_window_width_;
    stringstream(global_vars["min_window_height"]) >> min_window_height_;
    stringstream(global_vars["focused_color"]) >> hex >> focused_color_;
    stringstream(global_vars["unfocused_color"]) >> hex >> unfocused_color_;
}

Config::~Config() {}


int Config::GetSpawnWorkspaceId(const XClassHint& class_hint) const {
    string res_class = string(class_hint.res_class);
    string res_name = string(class_hint.res_name);

    if (spawn_rules_.find(res_class + ',' + res_name) != spawn_rules_.end()) {
        return spawn_rules_.at(res_class + ',' + res_name) - 1;
    } else if (spawn_rules_.find(res_class) != spawn_rules_.end()) {
        return spawn_rules_.at(res_class) - 1;
    } else {
        return WORKSPACE_ID_NULL;
    }
}

bool Config::ShouldFloat(const XClassHint& class_hint) const {
    string res_class = string(class_hint.res_class);
    string res_name = string(class_hint.res_name);

    if (float_rules_.find(res_class + ',' + res_name) != float_rules_.end()) {
        return float_rules_.at(res_class + ',' + res_name);
    } else if (float_rules_.find(res_class) != float_rules_.end()) {
        return float_rules_.at(res_class);
    } else {
        return false;
    }
}

bool Config::ShouldProhibit(const XClassHint& class_hint) const {
    string res_class = string(class_hint.res_class);
    string res_name = string(class_hint.res_name);

    if (prohibit_rules_.find(res_class + ',' + res_name) != prohibit_rules_.end()) {
        return prohibit_rules_.at(res_class + ',' + res_name);
    } else if (prohibit_rules_.find(res_class) != prohibit_rules_.end()) {
        return prohibit_rules_.at(res_class);
    } else {
        return false;
    }
}

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
    } else if (s == "prohibit") {
        return ConfigKeyword::PROHIBIT;
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


const unordered_map<string, short>& Config::spawn_rules() const {
    return spawn_rules_;
}

const unordered_map<string, bool>& Config::float_rules() const {
    return float_rules_;
}

const unordered_map<string, bool>& Config::prohibit_rules() const {
    return prohibit_rules_;
}

const unordered_map<string, vector<Action>>& Config::keybind_rules() const {
    return keybind_rules_;
}

const vector<string>& Config::autostart_rules() const {
    return autostart_rules_;
}

