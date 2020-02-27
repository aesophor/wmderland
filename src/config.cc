// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#include "config.h"

#include <fstream>
#include <sstream>

#include "action.h"
#include "log.h"
#include "util.h"

using std::ifstream;
using std::map;
using std::pair;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::vector;

namespace wmderland {

const vector<Action> Config::kEmptyActions_;

Config::Config(Display* dpy, Properties* prop, const string& filename)
    : dpy_(dpy), prop_(prop), filename_(sys_utils::ToAbsPath(filename)) {}

void Config::Load() {
  WM_LOG(INFO, "Loading user configuration: " << filename_);
  ifstream fin(filename_);
  fin >> *this;
}

int Config::GetSpawnWorkspaceId(Window window) const {
  for (const auto& key : GeneratePossibleConfigKeys(window)) {
    auto it = spawn_rules_.find(key);
    if (it != spawn_rules_.end()) {
      return it->second - 1;  // workspace id starts from 0.
    }
  }
  return UNSPECIFIED_WORKSPACE;
}

bool Config::ShouldFloat(Window window) const {
  for (const auto& key : GeneratePossibleConfigKeys(window)) {
    auto it = float_rules_.find(key);
    if (it != float_rules_.end()) {
      return it->second;
    }
  }
  return false;
}

bool Config::ShouldFullscreen(Window window) const {
  for (const auto& key : GeneratePossibleConfigKeys(window)) {
    auto it = fullscreen_rules_.find(key);
    if (it != fullscreen_rules_.end()) {
      return it->second;
    }
  }
  return false;
}

bool Config::ShouldProhibit(Window window) const {
  for (const auto& key : GeneratePossibleConfigKeys(window)) {
    auto it = prohibit_rules_.find(key);
    if (it != prohibit_rules_.end()) {
      return it->second;
    }
  }
  return false;
}

const vector<Action>& Config::GetKeybindActions(unsigned int modifier, KeyCode keycode) const {
  auto it = keybind_rules_.find({modifier, keycode});
  if (it != keybind_rules_.end()) {
    return it->second;
  }
  return Config::kEmptyActions_;
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

bool Config::focus_follows_mouse() const {
  return focus_follows_mouse_;
}

const map<pair<unsigned int, KeyCode>, vector<Action>>& Config::keybind_rules() const {
  return keybind_rules_;
}

const vector<string>& Config::autostart_cmds() const {
  return autostart_cmds_;
}

const vector<string>& Config::autostart_cmds_on_reload() const {
  return autostart_cmds_on_reload_;
}

Config::Keyword Config::StrToConfigKeyword(const std::string& s) {
  if (s == "set") {
    return Config::Keyword::SET;
  } else if (s == "assign") {
    return Config::Keyword::ASSIGN;
  } else if (s == "floating") {
    return Config::Keyword::FLOATING;
  } else if (s == "fullscreen") {
    return Config::Keyword::FULLSCREEN;
  } else if (s == "prohibit") {
    return Config::Keyword::PROHIBIT;
  } else if (s == "bindsym") {
    return Config::Keyword::BINDSYM;
  } else if (s == "exec" || s == "exec_on_reload") {
    return Config::Keyword::EXEC;
  } else {
    return Config::Keyword::UNDEFINED;
  }
}

string Config::ExtractWindowIdentifier(const std::string& s) {
  // Split the string with the first and the last whitespace,
  // and return the substring in the middle.
  string identifier = s.substr(s.find(' ') + 1);
  return identifier.substr(0, identifier.rfind(' '));
}

vector<string> Config::GeneratePossibleConfigKeys(Window window) const {
  pair<string, string> hint = wm_utils::GetXClassHint(window);
  const string& res_class = hint.first;
  const string& res_name = hint.second;
  string net_wm_name = wm_utils::GetNetWmName(window);

  return {
      res_class + ',' + res_name + ',' + net_wm_name,
      res_class + ',' + res_name,
      res_class + ',' + net_wm_name,
      res_class,
  };
}

const string& Config::ReplaceSymbols(string& s) {
  for (const auto& symtab_record : symtab_) {
    string_utils::Replace(s, symtab_record.first, symtab_record.second);
  }
  return s;
}

ifstream& operator>>(ifstream& ifs, Config& config) {
  // Load the built-in WM variables with their default values.
  config.gap_width_ = DEFAULT_GAP_WIDTH;
  config.border_width_ = DEFAULT_BORDER_WIDTH;
  config.min_window_width_ = MIN_WINDOW_WIDTH;
  config.min_window_height_ = MIN_WINDOW_HEIGHT;
  config.focused_color_ = DEFAULT_FOCUSED_COLOR;
  config.unfocused_color_ = DEFAULT_UNFOCUSED_COLOR;
  config.focus_follows_mouse_ = DEFAULT_FOCUS_FOLLOWS_MOUSE;

  config.symtab_.clear();
  config.spawn_rules_.clear();
  config.float_rules_.clear();
  config.fullscreen_rules_.clear();
  config.prohibit_rules_.clear();
  config.keybind_rules_.clear();
  config.autostart_cmds_.clear();
  config.autostart_cmds_on_reload_.clear();

  unordered_map<string, unsigned int> assignable_modifiers = {
      {"Mod1", Mod1Mask},        // Alt
      {"Mod2", Mod2Mask},        // NumLock
      {"Mod3", Mod3Mask},        // ScrollLock
      {"Mod4", Mod4Mask},        // Command/Windows
      {"Mod5", Mod5Mask},        // ?
      {"Shift", ShiftMask},      // Shift
      {"Control", ControlMask},  // Ctrl
  };

  // Parse user's config file.
  string line;
  while (std::getline(ifs, line)) {
    string_utils::Strip(line);  // Strip extra whitespace, just in case.

    if (line.empty() || line.front() == Config::kCommentSymbol) {
      continue;
    }

    vector<string> tokens = string_utils::Split(config.ReplaceSymbols(line), ' ');
    Config::Keyword keyword = Config::StrToConfigKeyword(tokens[0]);

    switch (keyword) {
      case Config::Keyword::SET: {
        string& key = tokens[1];
        string& value = tokens[3];
        if (string_utils::StartsWith(key, VARIABLE_PREFIX)) {
          // Prefixed with '$' means user-declared variable.
          config.symtab_[key] = value;
        } else {
          // Otherwise it is declaring value for a built-in variable.
          if (key == "gap_width") {
            config.gap_width_ = std::stoi(value);
          } else if (key == "border_width") {
            config.border_width_ = std::stoi(value);
          } else if (key == "min_window_width") {
            config.min_window_width_ = std::stoi(value);
          } else if (key == "min_window_height") {
            config.min_window_height_ = std::stoi(value);
          } else if (key == "focused_color") {
            config.focused_color_ = std::stoul(value, nullptr, 16);
          } else if (key == "unfocused_color") {
            config.unfocused_color_ = std::stoul(value, nullptr, 16);
          } else if (key == "focus_follows_mouse") {
            stringstream(tokens.back()) >> std::boolalpha >> config.focus_follows_mouse_;
          } else {
            WM_LOG(ERROR, "config: unrecognized identifier: " << key);
          }
        }
        break;
      }
      case Config::Keyword::ASSIGN: {
        string window_identifier = config.ExtractWindowIdentifier(line);
        config.spawn_rules_[window_identifier] = std::stoi(tokens.back());
        break;
      }
      case Config::Keyword::FLOATING: {
        string window_identifier = config.ExtractWindowIdentifier(line);
        stringstream(tokens.back()) >> std::boolalpha >>
            config.float_rules_[window_identifier];
        break;
      }
      case Config::Keyword::FULLSCREEN: {
        string window_identifier = config.ExtractWindowIdentifier(line);
        stringstream(tokens.back()) >> std::boolalpha >>
            config.fullscreen_rules_[window_identifier];
        break;
      }
      case Config::Keyword::PROHIBIT: {
        string window_identifier = config.ExtractWindowIdentifier(line);
        stringstream(tokens.back()) >> std::boolalpha >>
            config.prohibit_rules_[window_identifier];
        break;
      }
      case Config::Keyword::BINDSYM: {
        vector<string> keys = string_utils::Split(tokens[1], '+');
        unsigned int modifier = None;
        KeyCode keycode = None;

        for (const auto& key : keys) {
          auto it = assignable_modifiers.find(key);
          if (it != assignable_modifiers.end()) {  // key is a modifier
            modifier |= it->second;
          } else {  // key is a normal key, convert it to keysym
            keycode = XKeysymToKeycode(config.dpy_, XStringToKeysym(key.c_str()));
          }
        }

        string action_series_str = string_utils::Split(line, ' ', 2)[2];
        for (auto& action_str : string_utils::Split(action_series_str, ';')) {
          string_utils::Strip(action_str);
          config.keybind_rules_[{modifier, keycode}].push_back(Action(action_str));
          config.keybind_rules_[{modifier | LockMask, keycode}].push_back(Action(action_str));
        }
        break;
      }
      case Config::Keyword::EXEC: {
        string shell_cmd = string_utils::Split(line, ' ', 1)[1];
        config.autostart_cmds_.push_back(shell_cmd);

        if (tokens[0] == "exec_on_reload") {
          config.autostart_cmds_on_reload_.push_back(shell_cmd);
        }
        break;
      }
      default: {
        WM_LOG(ERROR, "config: unrecognized symbol: " << tokens[0]);
        break;
      }
    }
  }

  return ifs;
}

}  // namespace wmderland
