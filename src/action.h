// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_ACTION_H_
#define WMDERLAND_ACTION_H_

#include <string>

namespace wmderland {

class Action {
 public:
  enum class Type {
    NAVIGATE_LEFT,
    NAVIGATE_RIGHT,
    NAVIGATE_UP,
    NAVIGATE_DOWN,
    FLOAT_MOVE_LEFT,
    FLOAT_MOVE_RIGHT,
    FLOAT_MOVE_UP,
    FLOAT_MOVE_DOWN,
    FLOAT_RESIZE_LEFT,
    FLOAT_RESIZE_RIGHT,
    FLOAT_RESIZE_UP,
    FLOAT_RESIZE_DOWN,
    TILE_H,
    TILE_V,
    TOGGLE_FLOATING,
    TOGGLE_FULLSCREEN,
    GOTO_WORKSPACE,
    WORKSPACE,
    MOVE_WINDOW_TO_WORKSPACE,
    KILL,
    EXIT,
    RELOAD,
    DEBUG_CRASH,
    EXEC,
    UNDEFINED,
  };

  explicit Action(const std::string& s);
  explicit Action(Action::Type type);
  Action(Action::Type type, const std::string& argument);
  virtual ~Action() = default;

  Action::Type type() const;
  const std::string& argument() const;

 private:
  static Action::Type StrToActionType(const std::string& s);

  Action::Type type_;
  std::string argument_;
};

}  // namespace wmderland

#endif  // WMDERLAND_ACTION_H_
