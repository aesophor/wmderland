#include "action.hpp"
#include "util.hpp"

using std::string;
using std::vector;

Action::Action(const string& s) {
    vector<string> tokens = string_utils::Split(s, ' ', 1);
    type_ = Action::StrToActionType(tokens[0]);

    if (tokens.size() > 1) {
        arguments_ = tokens[1];
    }
}

Action::Action(ActionType type) : type_(type) {}

Action::Action(ActionType type, const string& arguments) 
    : type_(type), arguments_(arguments) {}

Action::~Action() {}


ActionType Action::type() const {
    return type_;
}

const string& Action::arguments() const {
    return arguments_;
}


ActionType Action::StrToActionType(const string& s) {
    if (s == "tile_horizontally") {
        return ActionType::TILE_H;
    } else if (s == "tile_vertically") {
        return ActionType::TILE_V;
    } else if (s == "focus_left") {
        return ActionType::FOCUS_LEFT;
    } else if (s == "focus_right") {
        return ActionType::FOCUS_RIGHT;
    } else if (s == "focus_down") {
        return ActionType::FOCUS_DOWN;
    } else if (s == "focus_up") {
        return ActionType::FOCUS_UP;
    } else if (s == "toggle_floating") {
        return ActionType::TOGGLE_FLOATING;
    } else if (s == "toggle_fullscreen") {
        return ActionType::TOGGLE_FULLSCREEN;
    } else if (s == "goto_workspace") {
        return ActionType::GOTO_WORKSPACE;
    } else if (s == "move_app_to_workspace") {
        return ActionType::MOVE_APP_TO_WORKSPACE;
    } else if (s == "kill") {
        return ActionType::KILL;
    } else if (s == "exit") {
        return ActionType::EXIT;
    } else if (s == "exec") {
        return ActionType::EXEC;
    } else {
        return ActionType::UNDEFINED;
    }
}
