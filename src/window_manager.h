// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#ifndef WMDERLAND_WINDOW_MANAGER_H_
#define WMDERLAND_WINDOW_MANAGER_H_

extern "C" {
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <array>
#include <memory>
#include <unordered_set>

#include "action.h"
#include "config.h"
#include "cookie.h"
#include "ipc.h"
#include "properties.h"
#include "snapshot.h"
#include "util.h"
#include "workspace.h"

namespace wmderland {

class WindowManager {
 public:
  static WindowManager* GetInstance();
  virtual ~WindowManager();

  void Run();
  void ArrangeWindows() const;

  Snapshot& snapshot();

 private:
  static WindowManager* instance_;
  static bool is_running_;
  WindowManager(Display* dpy);

  bool HasAnotherWmRunning();
  void InitXGrabs();
  void InitCursors();
  void InitProperties();
  void InitWorkspaces();

  // XEvent handlers
  void OnConfigureRequest(const XConfigureRequestEvent& e);
  void OnMapRequest(const XMapRequestEvent& e);
  void OnMapNotify(const XMapEvent& e);
  void OnUnmapNotify(const XUnmapEvent& e);
  void OnDestroyNotify(const XDestroyWindowEvent& e);
  void OnKeyPress(const XKeyEvent& e);
  void OnButtonPress(const XButtonEvent& e);
  void OnButtonRelease(const XButtonEvent& e);
  void OnMotionNotify(const XButtonEvent& e);
  void OnEnterNotify(const XEnterWindowEvent& e);
  void OnClientMessage(const XClientMessageEvent& e);
  void OnConfigReload();
  static int OnXError(Display* dpy, XErrorEvent* e);
  static int OnWmDetected(Display* dpy, XErrorEvent* e);

  void Manage(Window window);
  void Unmanage(Window window);
  void HandleAction(const Action& action);

  // Workspace manipulation
  void GotoWorkspace(int next);
  void MoveWindowToWorkspace(Window window, int next);

  // Client manipulation
  void SetFloating(Window window, bool floating, bool use_default_size);
  void SetFullscreen(Window window, bool fullscreen);
  void KillClient(Window window);

  // Docks, bars and notifications
  inline void MapDocks() const;
  inline void UnmapDocks() const;
  inline void RaiseNotifications() const;

  // Window position and size
  std::pair<int, int> GetDisplayResolution() const;
  Client::Area GetTilingArea() const;
  Client::Area GetFloatingWindowArea(Window window, bool use_default_size);

  // Misc
  void UpdateClientList();

  Display* dpy_;
  Window root_window_;
  Window wmcheckwin_;
  Cursor cursors_[4];

  std::unique_ptr<Properties> prop_;  // X and EWMH atoms
  std::unique_ptr<Config> config_;    // user config
  Cookie cookie_;                     // remembers pos/size of each window
  IpcEventManager ipc_evmgr_;         // client event manager
  Snapshot snapshot_;                 // error recovery

  // The floating windows unordered_set contains windows that should not be
  // tiled but must be kept on the top, e.g., dock, notifications, etc.
  std::unordered_set<Window> docks_;
  std::unordered_set<Window> notifications_;

  // Some programs (e.g., WPS office, Steam) might unmap its window(s)
  // but keep them in the background instead of destroying them. It is
  // up to the programs (i.e., owner of the windows) when to reuse them,
  // so we should not simply destroy these windows! We should store them
  // somewhere instead.
  std::unordered_set<Window> hidden_windows_;

  // Workspaces contain clients, where a client is a window that can be tiled
  // by the window manager.
  std::array<std::unique_ptr<Workspace>, WORKSPACE_COUNT> workspaces_;
  int current_;  // current workspace

  // Window move, resize event cache.
  XButtonEvent btn_pressed_event_;

  friend class IpcEventManager;
  friend class Snapshot;
};

}  // namespace wmderland

#endif  // WMDERLAND_WINDOW_MANAGER_H_
