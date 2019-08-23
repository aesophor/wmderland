// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "ipc.h"

extern "C" {
#include <X11/Xlib.h>
}

#include "config.h"
#include "client.h"
#include "window_manager.h"

namespace wmderland {

IpcEvent::IpcEvent(const XClientMessageEvent& e)
    : actionType(static_cast<Action::Type>(e.data.l[0])),
      argument(e.data.l[0]) {}


void IpcEventManager::Handle(const XClientMessageEvent& e) const {
  WindowManager* wm = WindowManager::GetInstance();
  if (!wm) {
    return;
  }

  IpcEvent ipc_event(e);
  Action action(ipc_event.actionType);

  if (ipc_event.argument) {
    action = Action(ipc_event.actionType, std::to_string(ipc_event.argument));
  }

  wm->HandleAction(action);
}

} // namespace wmderland
