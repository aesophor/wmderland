// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_ACTION_H_
#define WMDERLAND_ACTION_H_

#include <string>

namespace wmderland {

class Action {
 public:
  enum class Type {
    FOCUS_LEFT,
    FOCUS_RIGHT,
    FOCUS_DOWN,
    FOCUS_UP,
    TILE_H,
    TILE_V,
    TOGGLE_FLOATING,
    TOGGLE_FULLSCREEN,
    GOTO_WORKSPACE,
    MOVE_APP_TO_WORKSPACE,
    KILL,
    EXIT,
    EXEC,
    RELOAD,
    UNDEFINED
  };

  Action(const std::string& s);
  Action(Action::Type type);
  Action(Action::Type type, const std::string& argument);
  virtual ~Action() = default;

  Action::Type type() const;
  const std::string& argument() const;

 private:
  static Action::Type StrToActionType(const std::string& s);

  Action::Type type_;
  std::string argument_;
};

} // namespace wmderland

#endif // WMDERLAND_ACTION_H_
