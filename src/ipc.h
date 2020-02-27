// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_IPC_H_
#define WMDERLAND_IPC_H_

extern "C" {
#include <X11/Xlib.h>
}

#include "action.h"

namespace wmderland {

struct IpcEvent {
  explicit IpcEvent(const XClientMessageEvent& e);
  virtual ~IpcEvent() = default;

  const Action::Type actionType;
  bool has_argument;
  long argument;
};

class IpcEventManager {
 public:
  IpcEventManager() = default;
  virtual ~IpcEventManager() = default;

  void Handle(const XClientMessageEvent& e) const;
};

}  // namespace wmderland

#endif  // WMDERLAND_IPC_H_
