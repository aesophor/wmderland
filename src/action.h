#ifndef WMDERLAND_ACTION_H_
#define WMDERLAND_ACTION_H_

#include <string>
#include <vector>

enum class ActionType {
  TILE_H,
  TILE_V,
  FOCUS_LEFT,
  FOCUS_RIGHT,
  FOCUS_DOWN,
  FOCUS_UP,
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

class Action {
 public:
  Action(const std::string& s);
  Action(ActionType type);
  Action(ActionType type, const std::string& arguments);
  virtual ~Action() = default;

  ActionType type() const;
  const std::string& arguments() const;

 private:
  static ActionType StrToActionType(const std::string& s);

  ActionType type_;
  std::string arguments_;
};

#endif // WMDERLAND_ACTION_H_
