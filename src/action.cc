// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "action.h"

#include <vector>

#include "util.h"

using std::string;
using std::vector;

namespace wmderland {

Action::Action(const string& s) {
  // For example, "goto_workspace 1" is an action.
  // We split this string into two tokens by whitespace.
  vector<string> tokens = string_utils::Split(s, ' ', 1);

  // The first token is an action type.
  type_ = Action::StrToActionType(tokens[0]);

  // The second token (if exists) is an argument.
  if (tokens.size() > 1) {
    argument_ = tokens[1];
  }
}

Action::Action(Action::Type type) : type_(type), argument_() {}

Action::Action(Action::Type type, const string& argument) : type_(type), argument_(argument) {}

Action::Type Action::type() const {
  return type_;
}

const string& Action::argument() const {
  return argument_;
}

Action::Type Action::StrToActionType(const string& s) {
  if (s == "navigate_left") {
    return Action::Type::NAVIGATE_LEFT;
  } else if (s == "navigate_right") {
    return Action::Type::NAVIGATE_RIGHT;
  } else if (s == "navigate_up") {
    return Action::Type::NAVIGATE_UP;
  } else if (s == "navigate_down") {
    return Action::Type::NAVIGATE_DOWN;
  } else if (s == "float_move_left") {
    return Action::Type::FLOAT_MOVE_LEFT;
  } else if (s == "float_move_right") {
    return Action::Type::FLOAT_MOVE_RIGHT;
  } else if (s == "float_move_up") {
    return Action::Type::FLOAT_MOVE_UP;
  } else if (s == "float_move_down") {
    return Action::Type::FLOAT_MOVE_DOWN;
  } else if (s == "float_resize_left") {
    return Action::Type::FLOAT_RESIZE_LEFT;
  } else if (s == "float_resize_right") {
    return Action::Type::FLOAT_RESIZE_RIGHT;
  } else if (s == "float_resize_up") {
    return Action::Type::FLOAT_RESIZE_UP;
  } else if (s == "float_resize_down") {
    return Action::Type::FLOAT_RESIZE_DOWN;
  } else if (s == "tile_h") {
    return Action::Type::TILE_H;
  } else if (s == "tile_v") {
    return Action::Type::TILE_V;
  } else if (s == "toggle_floating") {
    return Action::Type::TOGGLE_FLOATING;
  } else if (s == "toggle_fullscreen") {
    return Action::Type::TOGGLE_FULLSCREEN;
  } else if (s == "goto_workspace") {
    return Action::Type::GOTO_WORKSPACE;
  } else if (s == "workspace") {
    return Action::Type::WORKSPACE;
  } else if (s == "move_window_to_workspace") {
    return Action::Type::MOVE_WINDOW_TO_WORKSPACE;
  } else if (s == "kill") {
    return Action::Type::KILL;
  } else if (s == "exit") {
    return Action::Type::EXIT;
  } else if (s == "reload") {
    return Action::Type::RELOAD;
  } else if (s == "debug_crash") {
    return Action::Type::DEBUG_CRASH;
  } else if (s == "exec") {
    return Action::Type::EXEC;
  } else {
    return Action::Type::UNDEFINED;
  }
}

}  // namespace wmderland
