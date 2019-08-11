// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_SNAPSHOT_H_
#define WMDERLAND_SNAPSHOT_H_

extern "C" {
#include <X11/Xlib.h>
}
#include <array>
#include <string>

#include "config.h"
#include "workspace.h"

namespace wmderland {

class Snapshot {
 public:
  Snapshot(Display* dpy, std::array<Workspace*, WORKSPACE_COUNT>& workspaces,
           const std::string& filename);
  virtual ~Snapshot() = default;

  bool FileExists() const;
  void Load() const;
  void Save() const;

 private:
  static const char kDelimiter_;

  Display* dpy_;
  std::array<Workspace*, WORKSPACE_COUNT>& workspaces_;
  const std::string filename_;
};

} // namespace wmderland

#endif // WMDERLAND_SNAPSHOT_H_
