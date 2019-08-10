// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "config.h"

#include <fstream>
#include <sstream>

#if GLOG_FOUND
#include <glog/logging.h>
#endif
#include "action.h"
#include "util.h"

using std::stoi;
using std::stoul;
using std::boolalpha;
using std::pair;
using std::string;
using std::vector;
using std::ifstream;
using std::stringstream;
using std::unordered_map;

namespace wmderland {

Config::Config(Display* dpy, Properties* prop, const string& filename)
    : dpy_(dpy), prop_(prop), filename_(filename) {
  Load();
}


void Config::Load() {
  // Convert it to full path first (in case it starts with ~)
  ifstream fin(sys_utils::ToAbsPath(filename_));
  fin >> *this;
  fin.close();
}

int Config::GetSpawnWorkspaceId(Window w) const {
  for (const auto& key : GeneratePossibleConfigKeys(w)) {
    if (spawn_rules_.find(key) != spawn_rules_.end()) {
      return spawn_rules_.at(key) - 1; // Workspace id starts from 0.
    }
  }
  return UNSPECIFIED_WORKSPACE;
}

bool Config::ShouldFloat(Window w) const {
  for (const auto& key : GeneratePossibleConfigKeys(w)) {
    if (float_rules_.find(key) != float_rules_.end()) {
      return float_rules_.at(key);
    }
  }
  return false;
}

bool Config::ShouldFullscreen(Window w) const {
  for (const auto& key: GeneratePossibleConfigKeys(w)) {
    if (fullscreen_rules_.find(key) != fullscreen_rules_.end()) {
      return fullscreen_rules_.at(key);
    }
  }
  return false;
}

bool Config::ShouldProhibit(Window w) const {
  for (const auto& key : GeneratePossibleConfigKeys(w)) {
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


unsigned int Config::gap_width() const {
  return gap_width_;
}

unsigned int Config::border_width() const {
  return border_width_;
}

unsigned int Config::min_window_width() const {
  return min_window_width_;
}

unsigned int Config::min_window_height() const {
  return min_window_height_;
}

unsigned long Config::focused_color() const {
  return focused_color_;
}

unsigned long Config::unfocused_color() const {
  return unfocused_color_;
}


const unordered_map<string, vector<Action>>& Config::keybind_rules() const {
  return keybind_rules_;
}

const vector<string>& Config::autostart_rules() const {
  return autostart_rules_;
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

vector<string> Config::GeneratePossibleConfigKeys(Window w) const {
  pair<string, string> hint = wm_utils::GetXClassHint(w);
  string& res_class = hint.first;
  string& res_name = hint.second;
  string net_wm_name = wm_utils::GetNetWmName(w);

  vector<string> keys;
  keys.push_back(res_class + ',' + res_name + ',' + net_wm_name);
  keys.push_back(res_class + ',' + res_name);
  keys.push_back(res_class + ',' + net_wm_name);
  keys.push_back(res_class);
  return keys;
}

const string& Config::ReplaceSymbols(string& s) {
  for (const auto& symtab_record : symtab_) {
    string_utils::Replace(s, symtab_record.first, symtab_record.second);
  }
  return s;
}


ifstream& operator>> (ifstream& ifs, Config& config) {
  // Load the built-in WM variables with their default values.
  config.gap_width_ = DEFAULT_GAP_WIDTH;
  config.border_width_ = DEFAULT_BORDER_WIDTH;
  config.min_window_width_ = MIN_WINDOW_WIDTH;
  config.min_window_height_ = MIN_WINDOW_HEIGHT;
  config.focused_color_ = DEFAULT_FOCUSED_COLOR;
  config.unfocused_color_ = DEFAULT_UNFOCUSED_COLOR;

  // Parse user's config file.
  string line;
  while (std::getline(ifs, line)) {
    string_utils::Strip(line); // Strip extra whitespace, just in case.

    if (!line.empty() && line.at(0) != Config::kCommentSymbol) {
      vector<string> tokens = string_utils::Split(config.ReplaceSymbols(line), ' ');
      ConfigKeyword keyword = Config::StrToConfigKeyword(tokens[0]);

      switch (keyword) {
        case ConfigKeyword::SET: {
          string& key = tokens[1];
          string& value = tokens[3];
          if (string_utils::StartsWith(key, VARIABLE_PREFIX)) {
            // Prefixed with '$' means user-declared variable.
            config.symtab_[key] = value;
          } else {
            // Otherwise it is declaring value for a built-in variable.
            if (key == "gap_width") {
              config.gap_width_ = stoi(value);
            } else if (key == "border_width") {
              config.border_width_ = stoi(value);
            } else if (key == "min_window_width") {
              config.min_window_width_ = stoi(value);
            } else if (key == "min_window_height") {
              config.min_window_height_ = stoi(value);
            } else if (key == "focused_color") {
              config.focused_color_ = stoul(value, nullptr, 16);
            } else if (key == "unfocused_color") {
              config.unfocused_color_ = stoul(value, nullptr, 16);
            } else {
              WM_LOG(ERROR, "config: unrecognized identifier: " << key);
            }
          }
          break;
        }
        case ConfigKeyword::ASSIGN: {
          string window_identifier = config.ExtractWindowIdentifier(line);
          config.spawn_rules_[window_identifier] = stoi(tokens.back());
          break;
        }
        case ConfigKeyword::FLOATING: {
          string window_identifier = config.ExtractWindowIdentifier(line);
          stringstream(tokens.back()) >> boolalpha >> config.float_rules_[window_identifier];
          break;
        }
        case ConfigKeyword::FULLSCREEN: {
          string window_identifier = config.ExtractWindowIdentifier(line);
          stringstream(tokens.back()) >> boolalpha >> config.fullscreen_rules_[window_identifier];
          break;
        }
        case ConfigKeyword::PROHIBIT: {
          string window_identifier = config.ExtractWindowIdentifier(line);
          stringstream(tokens.back()) >> boolalpha >> config.prohibit_rules_[window_identifier];
          break;
        }
        case ConfigKeyword::BINDSYM: {
          string modifier_and_key = tokens[1];
          string action_series_str = string_utils::Split(line, ' ', 2)[2];
          config.SetKeybindActions(modifier_and_key, action_series_str);
          break;
        }
        case ConfigKeyword::EXEC: {
          string shell_cmd = string_utils::Split(line, ' ', 1)[1];
          config.autostart_rules_.push_back(shell_cmd);
          break;
        }
        default: {
          WM_LOG(ERROR, "config: unrecognized symbol: " << tokens[0]);
          break;
        }
      }
    }
  }

  return ifs;
}

} // namespace wmderland
