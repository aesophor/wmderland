#include "action.hpp"
#include "config.hpp"
#include "util.hpp"
#include <fstream>
#include <sstream>
#if GLOG_FOUND
#include <glog/logging.h>
#endif

using std::hex;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;
using std::unordered_map;

Config::Config(Display* dpy, Properties* prop, const string& filename)
    : dpy_(dpy), prop_(prop), filename_(filename) {
    Load(filename);
}

Config::~Config() {}


void Config::Load(const string& filename) {
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
    std::ifstream file(sys_utils::ToAbsPath(filename));
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
                    break; }
                case ConfigKeyword::ASSIGN: {
                    string window_identifier = ExtractWindowIdentifier(line);
                    stringstream(tokens.back()) >> spawn_rules_[window_identifier];
                    break; }
                case ConfigKeyword::FLOATING: {
                    string window_identifier = ExtractWindowIdentifier(line);
                    stringstream(tokens.back()) >> std::boolalpha >> float_rules_[window_identifier];
                    break; }
                case ConfigKeyword::FULLSCREEN: {
                    string window_identifier = ExtractWindowIdentifier(line);
                    stringstream(tokens.back()) >> std::boolalpha >> fullscreen_rules_[window_identifier];
                    break; }
                case ConfigKeyword::PROHIBIT: {
                    string window_identifier = ExtractWindowIdentifier(line);
                    stringstream(tokens.back()) >> std::boolalpha >> prohibit_rules_[window_identifier];
                    break; }
                case ConfigKeyword::BINDSYM: {
                    string modifier_and_key = tokens[1];
                    string action_series_str = string_utils::Split(line, ' ', 2)[2];
                    SetKeybindActions(modifier_and_key, action_series_str);
                    break; }
                case ConfigKeyword::EXEC: {
                    string cmd = string_utils::Split(line, ' ', 1)[1];
                    autostart_rules_.push_back(cmd);
                    break; }
                default: {
                    WM_LOG(INFO, "Ignored unrecognized symbol in config: " << tokens[0]);
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

void Config::Reload() {
    Load(filename_);
}


int Config::GetSpawnWorkspaceId(Window w) const {
    for (auto& key : GeneratePossibleConfigKeys(w)) {
        if (spawn_rules_.find(key) != spawn_rules_.end()) {
            return spawn_rules_.at(key) - 1; // Workspace id starts from 0.
        }
    }
    return WORKSPACE_UNSPECIFIED;
}

bool Config::ShouldFloat(Window w) const {
    for (auto& key : GeneratePossibleConfigKeys(w)) {
        if (float_rules_.find(key) != float_rules_.end()) {
            return float_rules_.at(key);
        }
    }
    return false;
}

bool Config::ShouldFullscreen(Window w) const {
    for (auto& key: GeneratePossibleConfigKeys(w)) {
        if (fullscreen_rules_.find(key) != fullscreen_rules_.end()) {
            return fullscreen_rules_.at(key);
        }
    }
    return false;
}

bool Config::ShouldProhibit(Window w) const {
    for (auto& key : GeneratePossibleConfigKeys(w)) {
        if (prohibit_rules_.find(key) != prohibit_rules_.end()) {
            return prohibit_rules_.at(key);
        }
    }
    return false;
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


vector<string> Config::GeneratePossibleConfigKeys(Window w) const {
    pair<string, string> hint = wm_utils::GetXClassHint(dpy_, w);
    string& res_class = hint.first;
    string& res_name = hint.second;
    string net_wm_name = wm_utils::GetNetWmName(dpy_, w, prop_);

    vector<string> keys;
    keys.push_back(res_class + ',' + res_name + ',' + net_wm_name);
    keys.push_back(res_class + ',' + res_name);
    keys.push_back(res_class + ',' + net_wm_name);
    keys.push_back(res_class);

    return keys;
}

ConfigKeyword Config::StrToConfigKeyword(const std::string& s) {
    if (s == "set") {
        return ConfigKeyword::SET;
    } else if (s == "assign") {
        return ConfigKeyword::ASSIGN;
    } else if (s == "floating") {
        return ConfigKeyword::FLOATING;
    } else if (s == "fullscreen") {
        return ConfigKeyword::FULLSCREEN;
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

string Config::ExtractWindowIdentifier(const std::string& s) {
    // Split the string with the first and the last whitespace,
    // and return the substring in the middle.
    string identifier = string_utils::Split(s, ' ', 1)[1];
    identifier = identifier.substr(0, identifier.find_last_of(' '));
    return identifier;
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

const unordered_map<string, bool>& Config::fullscreen_rules() const {
    return fullscreen_rules_;
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

