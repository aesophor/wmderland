// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#include "ipc.h"

extern "C" {
#include <X11/Xlib.h>
}

#include "log.h"
#include "window_manager.h"

#define CMD_ID 0
#define HAS_ARGUMENT 1
#define ARGUMENT 2

namespace wmderland {

IpcEvent::IpcEvent(const XClientMessageEvent& e)
    : actionType(static_cast<Action::Type>(e.data.l[CMD_ID])),
      has_argument(static_cast<bool>(e.data.l[HAS_ARGUMENT])),
      argument(e.data.l[ARGUMENT]) {}

void IpcEventManager::Handle(const XClientMessageEvent& e) const {
  WindowManager* wm = WindowManager::GetInstance();

  if (!wm) {
    const char* err_msg = "IpcEventManager::Handle(), wm is nullptr!";
    WM_LOG(FATAL, err_msg);
    return;
  }


  IpcEvent ipc_event(e);
  Action action(ipc_event.actionType);

  if (ipc_event.has_argument) {
    action = Action(ipc_event.actionType, std::to_string(ipc_event.argument));
  }

  wm->HandleAction(action);
}

}  // namespace wmderland
