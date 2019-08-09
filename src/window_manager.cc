// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "window_manager.h"

extern "C" {
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
}
#include <memory>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>

#if GLOG_FOUND
#include <glog/logging.h>
#endif
#include "client.h"
#include "util.h"

#define MOUSE_LEFT_BTN 1
#define MOUSE_MID_BTN 2
#define MOUSE_RIGHT_BTN 3

#define NORMAL_CURSOR 0
#define MOVE_CURSOR 1
#define RESIZE_CURSOR 3

#define WM_STATE_WITHDRAWN 0
#define WM_STATE_NORMAL 1
#define WM_STATE_ICONIC 3

using std::pair;
using std::string;
using std::vector;

namespace wmderland {

WindowManager* WindowManager::instance_ = nullptr;

WindowManager* WindowManager::GetInstance() {
  // If the instance is not yet initialized, we'll try to open a display
  // to X server. If it fails (i.e., dpy is None), then we return nullptr
  // to the caller. Otherwise we return an instance of WindowManager.
  if (!instance_) {
    Display* dpy;
    instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : nullptr;
  }
  return instance_;
}

WindowManager::WindowManager(Display* dpy) 
    : dpy_(dpy), 
      root_window_(DefaultRootWindow(dpy_)),
      prop_(new Properties(dpy_)),
      config_(new Config(dpy_, prop_.get(), CONFIG_FILE)),
      cookie_(new Cookie(dpy_, prop_.get(), COOKIE_FILE)),
      current_(),
      btn_pressed_event_() {
  wm_utils::Init(dpy_, prop_.get(), root_window_);
  InitWorkspaces();
  InitProperties();
  InitXEvents();
  InitCursors();
}

WindowManager::~WindowManager() {
  XCloseDisplay(dpy_);

  for (const auto workspace : workspaces_) {
    delete workspace;
  }
}


void WindowManager::InitWorkspaces() {
  for (int i = 0; i < WORKSPACE_COUNT; i++) {
    workspaces_[i] = new Workspace(dpy_, root_window_, config_.get(), i);
  }
}

void WindowManager::InitProperties() {
  // Set the name of window manager (i.e., Wmderland) on the root_window_ window,
  // so that other programs can acknowledge the name of this WM.
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_WM_NAME], prop_->utf8string, 8,
      PropModeReplace, (unsigned char*) WIN_MGR_NAME, sizeof(WIN_MGR_NAME) / sizeof(char));

  // Supporting window for _NET_WM_SUPPORTING_CHECK which tells other client
  // a compliant window manager exists.
  wmcheckwin_ = XCreateSimpleWindow(dpy_, root_window_, 0, 0, 1, 1, 0, 0, 0);
  XChangeProperty(dpy_, wmcheckwin_, prop_->net[atom::NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
      PropModeReplace, (unsigned char *) &wmcheckwin_, 1);
  XChangeProperty(dpy_, wmcheckwin_, prop_->net[atom::NET_SUPPORTING_WM_CHECK], prop_->utf8string, 8,
      PropModeReplace, (unsigned char *) WIN_MGR_NAME, sizeof(WIN_MGR_NAME) / sizeof(char));
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_SUPPORTING_WM_CHECK], XA_WINDOW, 32,
      PropModeReplace, (unsigned char *) &wmcheckwin_, 1);

  // Initialize NET_CLIENT_LIST to empty.
  XDeleteProperty(dpy_, root_window_, prop_->net[atom::NET_CLIENT_LIST]);

  // Set _NET_SUPPORTED to indicate which atoms are supported by this window manager.    
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_SUPPORTED], XA_ATOM, 32,
      PropModeReplace, (unsigned char*) prop_->net, atom::NET_ATOM_SIZE);

  // Set _NET_NUMBER_OF_DESKTOP, _NET_CURRENT_DESKTOP, _NET_DESKTOP_VIEWPORT and _NET_DESKTOP_NAMES
  // to support polybar's xworkspace module.
  unsigned long workspace_count = WORKSPACE_COUNT;
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_NUMBER_OF_DESKTOPS], XA_CARDINAL, 32, 
      PropModeReplace, (unsigned char *) &workspace_count, 1);

  unsigned long current_workspace = current_;
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CURRENT_DESKTOP], XA_CARDINAL, 32, 
      PropModeReplace, (unsigned char *) &current_workspace, 1);

  unsigned long desktop_viewport_cord[2] = { 0, 0 };
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_DESKTOP_VIEWPORT], XA_CARDINAL, 32, 
      PropModeReplace, (unsigned char *) desktop_viewport_cord, 2);

  // TODO: Add/remove workspaces at runtime.
  // TODO: User should be able to modify workspace names at runtime.
  const char* names[WORKSPACE_COUNT] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
  XTextProperty text_prop;
  Xutf8TextListToTextProperty(dpy_, const_cast<char**>(names), WORKSPACE_COUNT, XUTF8StringStyle, &text_prop);
  XSetTextProperty(dpy_, root_window_, &text_prop, prop_->net[atom::NET_DESKTOP_NAMES]);
}

void WindowManager::InitXEvents() {
  // Define the key combinations which will send us X events based on the key combinations 
  // defined in config.
  for (const auto& r : config_->keybind_rules()) {
    vector<string> modifier_and_key = string_utils::Split(r.first, '+');
    bool shifted = string_utils::Contains(r.first, "Shift");

    string modifier = modifier_and_key[0];
    string key = modifier_and_key[(shifted) ? 2 : 1];

    int keycode = wm_utils::StrToKeycode(key);
    int mod_mask = wm_utils::StrToKeymask(modifier, shifted);
    XGrabKey(dpy_, keycode, mod_mask, root_window_, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy_, keycode, mod_mask | LockMask, root_window_, True, GrabModeAsync, GrabModeAsync);
  }

  // Define which mouse clicks will send us X events.
  XGrabButton(dpy_, AnyButton, Mod4Mask, root_window_, True,
      ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

  // Enable substructure redirection on the root window.
  XSelectInput(dpy_, root_window_, SubstructureNotifyMask | SubstructureRedirectMask);

  // Setup the bitch catcher.
  XSetErrorHandler(&WindowManager::OnXError);
}

void WindowManager::InitCursors() {
  cursors_[NORMAL_CURSOR] = XCreateFontCursor(dpy_, XC_left_ptr);
  cursors_[RESIZE_CURSOR] = XCreateFontCursor(dpy_, XC_sizing);
  cursors_[MOVE_CURSOR] = XCreateFontCursor(dpy_, XC_fleur);
  XDefineCursor(dpy_, root_window_, cursors_[NORMAL_CURSOR]);
}

void WindowManager::Run() {
  // Automatically start the applications specified in config in background.
  for (const auto& s : config_->autostart_rules()) {
    system((s + '&').c_str());
  }

  XEvent event;
  for (;;) {
    // Retrieve and dispatch next X event.
    XNextEvent(dpy_, &event);

    switch (event.type) {
      case ConfigureRequest:
        OnConfigureRequest(event.xconfigurerequest);
        break;
      case MapRequest:
        OnMapRequest(event.xmaprequest);
        break;
      case MapNotify:
        OnMapNotify(event.xmap);
        break;
      case DestroyNotify:
        OnDestroyNotify(event.xdestroywindow);
        break;
      case KeyPress:
        OnKeyPress(event.xkey);
        break;
      case ButtonPress:
        OnButtonPress(event.xbutton);
        break;
      case ButtonRelease:
        OnButtonRelease(event.xbutton);
        break;
      case MotionNotify:
        OnMotionNotify(event.xbutton);
        break;
      case ClientMessage:
        OnClientMessage(event.xclient);
        break;
      default:
        break;
    }
  }
}


void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e) {
  XWindowChanges changes;
  changes.x = e.x;
  changes.y = e.y;
  changes.width = e.width;
  changes.height = e.height;
  changes.border_width = e.border_width;
  changes.sibling = e.above;
  changes.stack_mode = e.detail;
  XConfigureWindow(dpy_, e.window, e.value_mask, &changes);
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
  // If user has requested to prohibit this window from being mapped, then don't map it.
  if (config_->ShouldProhibit(e.window)) {
    return;
  }

  wm_utils::SetWindowWmState(e.window, WM_STATE_NORMAL);

  // If this window is a dock (or bar), map it, add it to docks_
  // and arrange the workspace.
  if (wm_utils::IsDock(e.window)
      && std::find(docks_.begin(), docks_.end(), e.window) == docks_.end()) {
    XMapWindow(dpy_, e.window);
    docks_.push_back(e.window);
    workspaces_[current_]->Arrange(CalculateTilingArea());
    return;
  }
  
  // If this window is wine steam dialog, just map it directly and don't manage it.
  pair<string, string> hint = wm_utils::GetXClassHint(e.window);
  if (wm_utils::IsDialog(e.window)) {
    if (hint.first == "Wine" && hint.second == "steam.exe") {
      XMapWindow(dpy_, e.window);
      return;
    }
  }

  // Pass all checks above -> we should manage this window.
  // Spawn this window in the specified workspace if such rule exists,
  // otherwise spawn it in current workspace.
  int target = config_->GetSpawnWorkspaceId(e.window);

  if (target == UNSPECIFIED_WORKSPACE) {
    target = current_;
  }


  // If this window is already in this workspace, don't add it to this workspace again.
  if (workspaces_[target]->Has(e.window)) {
    return;
  }

  bool should_float = config_->ShouldFloat(e.window)
    || wm_utils::IsDialog(e.window)
    || wm_utils::IsSplash(e.window)
    || wm_utils::IsUtility(e.window);

  bool should_fullscreen = config_->ShouldFullscreen(e.window)
    || wm_utils::HasNetWmStateFullscreen(e.window);

  Client* prev_focused_client = workspaces_[target]->GetFocusedClient();
  workspaces_[target]->UnsetFocusedClient();
  workspaces_[target]->Add(e.window, should_float);
  UpdateClientList(); // update NET_CLIENT_LIST

  if (workspaces_[target]->is_fullscreen()) {
    workspaces_[target]->SetFocusedClient(prev_focused_client->window());
  }

  if (target == current_ && !workspaces_[current_]->is_fullscreen()) {
    // If there's currently a fullscreen window, don't arrange the workspace,
    // or the entire workspace will mess up.
    workspaces_[current_]->GetClient(e.window)->Map();
    workspaces_[current_]->Arrange(CalculateTilingArea());
    workspaces_[current_]->SetFocusedClient(e.window);
    workspaces_[current_]->RaiseAllFloatingClients();
    RaiseNotifications();
    wm_utils::SetNetActiveWindow(e.window);
  }

  if (should_float) {
    DetermineFloatingWindowArea(e.window);
  }

  if (should_fullscreen) {
    SetFullscreen(e.window, true);
  }
}

void WindowManager::OnMapNotify(const XMapEvent& e) {
  // Checking if a window is a notification in OnMapRequest() will fail (especially dunst),
  // So we perform the check here (after the window is mapped) instead.
  if (wm_utils::IsNotification(e.window) 
      && std::find(notifications_.begin(), notifications_.end(), e.window) == notifications_.end()) {
    notifications_.push_back(e.window);
  }
}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {
  // If the window is a dock/bar or notification, remove it and tile the workspace.
  if (std::find(docks_.begin(), docks_.end(), e.window) != docks_.end()) {
    docks_.erase(remove(docks_.begin(), docks_.end(), e.window), docks_.end());
    workspaces_[current_]->Arrange(CalculateTilingArea());
    return;
  } else if (wm_utils::IsNotification(e.window)) {
    notifications_.erase(remove(notifications_.begin(), notifications_.end(), e.window), notifications_.end());
    return;
  }

  // Set window state to withdrawn (wine application needs this to destroy windows properly).
  // TODO: Wine steam floating menu still laggy upon removal
  wm_utils::SetWindowWmState(e.window, WM_STATE_WITHDRAWN);

  // If we aren't managing this window, there's no need to proceed further.
  Client* c = Client::mapper_[e.window];
  if (!c) {
    return;
  }

  // If the client being destroyed is in fullscreen mode, make sure to unset the workspace's
  // fullscreen state.
  if (c->is_fullscreen()) {
    c->workspace()->set_fullscreen(false);
    c->workspace()->MapAllClients();
    MapDocks();
  }

  // Remove the corresponding client from the client tree.
  c->workspace()->Remove(e.window);
  UpdateClientList();

  // Clear _NET_ACTIVE_WINDOW only if the window being destroyed is in current workspace.
  if (c->workspace() == workspaces_[current_]) {
    wm_utils::ClearNetActiveWindow();
  }

  // Transfer focus to another window (if there's still one).
  Client* new_focused_client = c->workspace()->GetFocusedClient();
  if (c->workspace() == workspaces_[current_] && new_focused_client) {
    c->workspace()->SetFocusedClient(new_focused_client->window());
    c->workspace()->Arrange(CalculateTilingArea());
    c->workspace()->RaiseAllFloatingClients();
    RaiseNotifications();
    wm_utils::SetNetActiveWindow(new_focused_client->window());
  }
}

void WindowManager::OnKeyPress(const XKeyEvent& e) {
  Client* focused_client = workspaces_[current_]->GetFocusedClient();

  const vector<Action>& actions = config_->GetKeybindActions(
      wm_utils::KeymaskToStr(e.state), // modifier str, e.g., "Mod4+Shift"
      wm_utils::KeysymToStr(e.keycode) // key str, e.g., "q"
  );

  for (const auto& action : actions) {
    switch (action.type()) {
      case Action::Type::FOCUS_LEFT:
      case Action::Type::FOCUS_RIGHT:
      case Action::Type::FOCUS_UP:
      case Action::Type::FOCUS_DOWN:
        if (!focused_client || workspaces_[current_]->is_fullscreen()) continue;
        workspaces_[current_]->Focus(action.type());
        workspaces_[current_]->RaiseAllFloatingClients();
        RaiseNotifications();
        wm_utils::SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
        break;
      case Action::Type::TILE_H:
        workspaces_[current_]->SetTilingDirection(TilingDirection::HORIZONTAL);
        break;
      case Action::Type::TILE_V:
        workspaces_[current_]->SetTilingDirection(TilingDirection::VERTICAL);
        break;
      case Action::Type::TOGGLE_FLOATING:
        if (!focused_client) continue;
        SetFloating(focused_client->window(), !focused_client->is_floating());
        break;
      case Action::Type::TOGGLE_FULLSCREEN:
        if (!focused_client) continue;
        SetFullscreen(focused_client->window(), !focused_client->is_fullscreen());
        break;
      case Action::Type::GOTO_WORKSPACE:
        GotoWorkspace(std::stoi(action.argument()) - 1);
        break;
      case Action::Type::MOVE_APP_TO_WORKSPACE:
        if (!focused_client) continue;
        MoveWindowToWorkspace(focused_client->window(), std::stoi(action.argument()) - 1);
        break;
      case Action::Type::KILL:
        if (!focused_client) continue;
        KillClient(focused_client->window());
        break;
      case Action::Type::EXIT:
        system("pkill X");
        break;
      case Action::Type::EXEC:
        system((action.argument() + '&').c_str());
        break;
      case Action::Type::RELOAD:
        config_->Load();
        OnConfigReload();
        break;
      default:
        break;
    }
  }
}

void WindowManager::OnButtonPress(const XButtonEvent& e) {
  Client* c = Client::mapper_[e.subwindow];
  if (!e.subwindow || !c) {
    return;
  }

  c->workspace()->UnsetFocusedClient();
  c->workspace()->SetFocusedClient(c->window());
  c->workspace()->RaiseAllFloatingClients();
  wm_utils::SetNetActiveWindow(c->window());

  if (c->is_floating() && !c->is_fullscreen()) {
    c->SaveXWindowAttributes();
    btn_pressed_event_ = e;
    XDefineCursor(dpy_, root_window_, cursors_[e.button]);
  }
}

void WindowManager::OnButtonRelease(const XButtonEvent& e) {
  if (!btn_pressed_event_.subwindow || !e.subwindow) {
    return;
  }

  Client* c = Client::mapper_[btn_pressed_event_.subwindow];
  if (c && c->is_floating()) {
    XWindowAttributes attr = wm_utils::GetXWindowAttributes(btn_pressed_event_.subwindow);
    cookie_->Put(c->window(), {attr.x, attr.y, attr.width, attr.height});
  }

  btn_pressed_event_.subwindow = None;
  XDefineCursor(dpy_, root_window_, cursors_[NORMAL_CURSOR]);
}

void WindowManager::OnMotionNotify(const XButtonEvent& e) {
  Client* c = Client::mapper_[btn_pressed_event_.subwindow];
  if (!btn_pressed_event_.subwindow || !c) {
    return;
  }

  const XWindowAttributes& attr = c->previous_attr();
  int xdiff = e.x - btn_pressed_event_.x;
  int ydiff = e.y - btn_pressed_event_.y;
  int new_x = attr.x + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? xdiff : 0);
  int new_y = attr.y + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? ydiff : 0);
  int new_width = attr.width + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? xdiff : 0);
  int new_height = attr.height + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? ydiff : 0);

  int min_width = (c->size_hints().min_width > 0) ? c->size_hints().min_width : MIN_WINDOW_WIDTH;
  int min_height = (c->size_hints().min_height > 0) ? c->size_hints().min_height : MIN_WINDOW_HEIGHT;
  new_width = (new_width < min_width) ? min_width : new_width;
  new_height = (new_height < min_height) ? min_height : new_height;
  c->MoveResize(new_x, new_y, new_width, new_height);
}

void WindowManager::OnClientMessage(const XClientMessageEvent& e) {
  if (e.message_type == prop_->net[atom::NET_CURRENT_DESKTOP]) {
    if (e.data.l[0] >= 0 && e.data.l[0] < WORKSPACE_COUNT) {
      GotoWorkspace(e.data.l[0]);
    }
  } else if (e.message_type == prop_->wmderland_client_event) {

  }
}

void WindowManager::OnConfigReload() {
  // 1. Re-arrange all workspaces.
  // 2. Apply new border width and color to existing clients.
  Area tiling_area = CalculateTilingArea();

  for (const auto workspace : workspaces_) {
    workspace->Arrange(tiling_area);

    for (const auto client : workspace->GetClients()) {
      client->SetBorderWidth(config_->border_width());
      client->SetBorderColor(config_->unfocused_color());
    }

    Client* focused_client = workspace->GetFocusedClient();
    if (focused_client) {
      focused_client->SetBorderColor(config_->focused_color());
    }
  }
}

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
  #if GLOG_FOUND
  const int MAX_ERROR_TEXT_LENGTH = 1024;
  char error_text[MAX_ERROR_TEXT_LENGTH];
  XGetErrorText(dpy, e->error_code, error_text, sizeof(error_text));
  LOG(ERROR) << "Received X error:\n"
    << "    Request: " << int(e->request_code)
    << "    Error code: " << int(e->error_code)
    << " - " << error_text << "\n"
    << "    Resource ID: " << e->resourceid;
  return 0; // The return value is ignored.
  #endif
}


void WindowManager::ArrangeWindows() const {
  workspaces_[current_]->MapAllClients();
  workspaces_[current_]->Arrange(CalculateTilingArea());
  workspaces_[current_]->RaiseAllFloatingClients();
  RaiseNotifications();
  MapDocks();
  
  Client* focused_client = workspaces_[current_]->GetFocusedClient();

  if (!focused_client) {
    wm_utils::ClearNetActiveWindow();
  } else {
    // Update NET_ACTIVE_WINDOW
    wm_utils::SetNetActiveWindow(focused_client->window());

    // Make sure the focused client is receiving input focus.
    workspaces_[current_]->SetFocusedClient(focused_client->window());

    // Restore fullscreen application.
    if (workspaces_[current_]->is_fullscreen()) {
      focused_client->MoveResize(0, 0, GetDisplayResolution());
      UnmapDocks();
    }
  }
}

void WindowManager::GotoWorkspace(int next) {
  if (current_ == next) {
    return;
  }

  workspaces_[current_]->UnmapAllClients();
  workspaces_[next]->MapAllClients();
  current_ = next;
  ArrangeWindows();

  // Update _NET_CURRENT_DESKTOP
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CURRENT_DESKTOP], XA_CARDINAL, 32, 
      PropModeReplace, reinterpret_cast<uint8_t*>(&next), 1);
}

void WindowManager::MoveWindowToWorkspace(Window window, int next) {    
  Client* c = Client::mapper_[window];
  if (current_ == next || !c){
    return;
  }

  if (workspaces_[current_]->is_fullscreen()) {
    workspaces_[current_]->set_fullscreen(false);
    c->set_fullscreen(false);
    c->workspace()->MapAllClients();
    MapDocks();
  }

  c->Unmap();
  workspaces_[next]->UnsetFocusedClient();
  workspaces_[current_]->Move(window, workspaces_[next]);
  ArrangeWindows();
}


void WindowManager::Center(Window window) {
  pair<short, short> res = GetDisplayResolution();
  short screen_width = res.first;
  short screen_height = res.second;

  XWindowAttributes attr = wm_utils::GetXWindowAttributes(window);
  int new_x = screen_width / 2 - attr.width / 2;
  int new_y = screen_height / 2 - attr.height / 2;
  XMoveWindow(dpy_, window, new_x, new_y);
}

void WindowManager::SetFloating(Window window, bool floating) {
  Client* c = Client::mapper_[window];
  if (!c || c->is_fullscreen()) {
    return;
  }

  if (floating) {
    c->Resize(DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
    Center(c->window());
  }

  c->set_floating(floating);
  c->workspace()->Arrange(CalculateTilingArea());
}

void WindowManager::SetFullscreen(Window window, bool fullscreen) {
  Client* c = Client::mapper_[window];
  if (!c || c->is_fullscreen() == fullscreen) {
    return;
  }

  c->set_fullscreen(fullscreen);
  c->workspace()->set_fullscreen(fullscreen);
  c->SetBorderWidth((fullscreen) ? 0 : config_->border_width());

  if (fullscreen) {
    UnmapDocks();
    c->SaveXWindowAttributes();
    c->MoveResize(0, 0, GetDisplayResolution());
    c->workspace()->UnmapAllClients();
    c->Map();
    c->workspace()->SetFocusedClient(c->window());
  } else {
    MapDocks();
    const XWindowAttributes& attr = c->previous_attr();
    c->MoveResize(attr.x, attr.y, attr.width, attr.height);
    ArrangeWindows();
  }
 
  // Update window's _NET_WM_STATE_FULLSCREEN property.
  // If the window is set to be NOT fullscreen, we will simply write a nullptr with 0 elements.
  Atom* atom = (fullscreen) ? &prop_->net[atom::NET_WM_STATE_FULLSCREEN] : nullptr;
  XChangeProperty(dpy_, window, prop_->net[atom::NET_WM_STATE], XA_ATOM, 32,
      PropModeReplace, reinterpret_cast<uint8_t*>(atom), fullscreen);
}

void WindowManager::KillClient(Window window) {
  Atom* supported_protocols;
  int num_supported_protocols;

  // First try to kill the client gracefully via ICCCM. If the client does not support
  // this method, then we'll perform the brutal XKillClient().
  if (XGetWMProtocols(dpy_, window, &supported_protocols, &num_supported_protocols) 
      && (std::find(supported_protocols, supported_protocols + num_supported_protocols, 
          prop_->wm[atom::WM_DELETE]) != supported_protocols + num_supported_protocols)) {
    XEvent msg;
    memset(&msg, 0, sizeof(msg));
    msg.xclient.type = ClientMessage;
    msg.xclient.message_type = prop_->wm[atom::WM_PROTOCOLS];
    msg.xclient.window = window;
    msg.xclient.format = 32;
    msg.xclient.data.l[0] = prop_->wm[atom::WM_DELETE];
    XSendEvent(dpy_, window, false, 0, &msg);
  } else {
    XKillClient(dpy_, window);
  }
}


inline void WindowManager::MapDocks() const {
  for (const auto window : docks_) {
    XMapWindow(dpy_, window);
  }
}

inline void WindowManager::UnmapDocks() const {
  for (const auto window : docks_) {
    XUnmapWindow(dpy_, window);
  }
}

inline void WindowManager::RaiseNotifications() const {
  for (const auto window : notifications_) {
    XRaiseWindow(dpy_, window);
  }
}


pair<int, int> WindowManager::GetDisplayResolution() const {
  XWindowAttributes root_window_attr = wm_utils::GetXWindowAttributes(root_window_);
  return {root_window_attr.width, root_window_attr.height};
}

Area WindowManager::CalculateTilingArea() const {
  pair<int, int> res = GetDisplayResolution();
  Area tiling_area = {0, 0, res.first, res.second};

  for (const auto window : docks_) {
    XWindowAttributes dock_attr = wm_utils::GetXWindowAttributes(window);

    if (dock_attr.y == 0) {
      // If the dock is at the top of the screen.
      tiling_area.y += dock_attr.height;
      tiling_area.height -= dock_attr.height;
    } else if (dock_attr.y + dock_attr.height == tiling_area.y + tiling_area.height) {
      // If the dock is at the bottom of the screen.
      tiling_area.height -= dock_attr.height;
    } else if (dock_attr.x == 0) {
      // If the dock is at the leftmost of the screen.
      tiling_area.x += dock_attr.width;
      tiling_area.width -= dock_attr.width;
    } else if (dock_attr.x + dock_attr.width == tiling_area.x + tiling_area.width) {
      // If the dock is at the rightmost of the screen.
      tiling_area.width -= dock_attr.width;
    }
  }

  return tiling_area;
}

void WindowManager::DetermineFloatingWindowArea(Window window) {
  XSizeHints hints = wm_utils::GetWmNormalHints(window);
  const Area& cookie_area = cookie_->Get(window);

  // Set window size. (Priority: cookie > hints)
  if (cookie_area.width > 0 && cookie_area.height > 0) {
    XResizeWindow(dpy_, window, cookie_area.width, cookie_area.height);
  } else if (hints.min_width > 0 && hints.min_height > 0) {
    XResizeWindow(dpy_, window, hints.min_width, hints.min_height);
  } else if (hints.base_width > 0 && hints.base_height > 0) {
    XResizeWindow(dpy_, window, hints.base_width, hints.base_height);
  } else if (hints.width > 0 && hints.height > 0) {
    XResizeWindow(dpy_, window, hints.width, hints.height);
  } else {
    XResizeWindow(dpy_, window, DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
  }

  // Set window position. (Priority: cookie > hints)
  if (cookie_area.x > 0 && cookie_area.y > 0) {
    XMoveWindow(dpy_, window, cookie_area.x, cookie_area.y);
  } else if (hints.x > 0 && hints.y > 0) {
    XMoveWindow(dpy_, window, hints.x, hints.y);
  } else {
    Center(window);
  }
}


void WindowManager::UpdateClientList() {
  XDeleteProperty(dpy_, root_window_, prop_->net[atom::NET_CLIENT_LIST]);

  for (const auto workspace : workspaces_) {
    for (const auto client : workspace->GetClients()) {
      Window window = client->window();
      XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CLIENT_LIST], XA_WINDOW, 32, 
          PropModeAppend, reinterpret_cast<uint8_t*>(&window), 1);
    }
  }
}

} // namespace wmderland
