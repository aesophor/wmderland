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
using std::unique_ptr;
using std::stringstream;
using std::unordered_map;
using wmderland::tiling::Direction;
using wmderland::wm_utils::IsDock;
using wmderland::wm_utils::IsDialog;
using wmderland::wm_utils::IsSplash;
using wmderland::wm_utils::IsUtility;
using wmderland::wm_utils::IsNotification;

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
      current_(0),
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
  Xutf8TextListToTextProperty(dpy_, (char**) names, WORKSPACE_COUNT, XUTF8StringStyle, &text_prop);
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
  // Windows that are prohibited from mapping should be killed immediately.
  if (config_->ShouldProhibit(e.window)) {
    XKillClient(dpy_, e.window);
    return;
  }

  // If this window is a dock (or bar), record it, map it and tile the workspace.
  if (IsDock(e.window) && std::find(docks_.begin(), docks_.end(), e.window) == docks_.end()) {
    docks_.push_back(e.window);
    workspaces_[current_]->Arrange(CalculateTilingArea());
    XMapWindow(dpy_, e.window);
    return;
  }

  wm_utils::SetWindowWmState(e.window, WM_STATE_NORMAL);

  // If this window is wine steam dialog, just map it directly and don't manage it.
  pair<string, string> hint = wm_utils::GetXClassHint(e.window);
  if (IsDialog(e.window)) {
    if (hint.first == "Wine" && hint.second == "steam.exe") {
      XMapWindow(dpy_, e.window);
      return;
    }
  }


  // Pass all checks above -> we should manage this window.
  // Floating criteria: has _NET_WM_WINDOW_TYPE_{DIALOG,SPLASH,UTILITY} or is requested in config.
  bool should_float = IsDialog(e.window) || IsSplash(e.window) || IsUtility(e.window) || config_->ShouldFloat(e.window);

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

  Client* prev_focused_client = workspaces_[target]->GetFocusedClient();
  workspaces_[target]->UnsetFocusedClient();
  workspaces_[target]->Add(e.window, should_float);
  Client* current_focused_client = workspaces_[target]->GetClient(e.window);

  if (workspaces_[target]->is_fullscreen()) {
    workspaces_[target]->SetFocusedClient(prev_focused_client->window());
  }

  if (target == current_) {
    // If there's currently a fullscreen window, don't arrange the workspace,
    // or the entire workspace will mess up.
    if (!workspaces_[target]->is_fullscreen()) {
      current_focused_client->Map();
      workspaces_[target]->Arrange(CalculateTilingArea());
      workspaces_[target]->SetFocusedClient(e.window);
      workspaces_[target]->RaiseAllFloatingClients();
      RaiseNotifications();
      wm_utils::SetNetActiveWindow(e.window);
    }
  }

  if (should_float) {
    DetermineFloatingWindowArea(e.window);
  }

  // If the window we're launching has _NET_WM_STATE_FULLSCREEN, or is requested in config,
  // then we should set this window to fullscreen.
  if (config_->ShouldFullscreen(e.window) || wm_utils::HasNetWmStateFullscreen(e.window)) {
    SetFullscreen(e.window, true);
  }

  // Update NET_CLIENT_LIST
  UpdateClientList();
}

void WindowManager::OnMapNotify(const XMapEvent& e) {
  // Checking if a window is a notification in OnMapRequest() will fail (especially dunst),
  // So we perform the check here (after the window is mapped) instead.
  if (IsNotification(e.window) 
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
  } else if (IsNotification(e.window)) {
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
      case Action::Type::TILE_H:
        workspaces_[current_]->SetTilingDirection(Direction::HORIZONTAL);
        break;
      case Action::Type::TILE_V:
        workspaces_[current_]->SetTilingDirection(Direction::VERTICAL);
        break;
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
  if (!btn_pressed_event_.subwindow) {
    return;
  }

  Client* c = Client::mapper_[btn_pressed_event_.subwindow];
  if (!c) {
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
  // 1. Rearrange all workspaces.
  // 2. Apply new border width and color to existing clients.
  Area tiling_area = CalculateTilingArea();

  for (auto workspace : workspaces_) {
    workspace->Arrange(tiling_area);

    for (auto client : workspace->GetClients()) {
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


void WindowManager::GotoWorkspace(int next) {
  if (current_ == next) {
    return;
  }

  MapDocks();
  wm_utils::ClearNetActiveWindow();

  workspaces_[current_]->UnmapAllClients();
  workspaces_[next]->MapAllClients();
  workspaces_[next]->RaiseAllFloatingClients();
  current_ = next;

  Client* focused_client = workspaces_[next]->GetFocusedClient();
  if (focused_client) {
    workspaces_[next]->SetFocusedClient(focused_client->window());
    workspaces_[next]->Arrange(CalculateTilingArea());
    workspaces_[next]->RaiseAllFloatingClients();
    RaiseNotifications();
    wm_utils::SetNetActiveWindow(focused_client->window());

    // Restore fullscreen application.
    if (workspaces_[next]->is_fullscreen()) {
      focused_client->MoveResize(0, 0, wm_utils::GetDisplayResolution());
      UnmapDocks();
    }
  }

  // Update _NET_CURRENT_DESKTOP
  unsigned long current_workspace = current_;
  XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CURRENT_DESKTOP], XA_CARDINAL, 32, 
      PropModeReplace, (unsigned char *) &current_workspace, 1);
}

void WindowManager::MoveWindowToWorkspace(Window window, int next) {    
  if (current_ == next){
    return;
  }

  Client* c = Client::mapper_[window];
  if (!c) {
    return;
  }

  if (workspaces_[current_]->is_fullscreen()) {
    workspaces_[current_]->set_fullscreen(false);
    c->set_fullscreen(false);
    c->workspace()->MapAllClients();
    MapDocks();
  }

  c->Unmap();
  Client* next_workspace_prev_focused_client = workspaces_[next]->GetFocusedClient();
  workspaces_[next]->UnsetFocusedClient();
  workspaces_[current_]->Move(window, workspaces_[next]);

  if (workspaces_[next]->is_fullscreen()) {
    workspaces_[next]->SetFocusedClient(next_workspace_prev_focused_client->window());
  } else {
    workspaces_[next]->UnsetFocusedClient();
    workspaces_[next]->SetFocusedClient(window);
  }

  Client* current_workspace_focused_client = workspaces_[current_]->GetFocusedClient();
  if (current_workspace_focused_client) {
    workspaces_[current_]->Arrange(CalculateTilingArea());
    workspaces_[current_]->SetFocusedClient(current_workspace_focused_client->window());
    c->workspace()->RaiseAllFloatingClients();
    RaiseNotifications();
  } else {
    wm_utils::ClearNetActiveWindow();
  }
}


void WindowManager::Center(Window w) {
  pair<short, short> res = wm_utils::GetDisplayResolution();
  short screen_width = res.first;
  short screen_height = res.second;

  XWindowAttributes attr = wm_utils::GetXWindowAttributes(w);
  int new_x = screen_width / 2 - attr.width / 2;
  int new_y = screen_height / 2 - attr.height / 2;
  XMoveWindow(dpy_, w, new_x, new_y);
}

void WindowManager::SetFloating(Window w, bool is_floating) {
  Client* c = Client::mapper_[w];
  if (!c || c->is_fullscreen()) {
    return;
  }

  if (is_floating) {
    c->Resize(DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
    Center(c->window());
  }

  c->set_floating(!c->is_floating());
  c->workspace()->Arrange(CalculateTilingArea());
}

void WindowManager::SetFullscreen(Window w, bool is_fullscreen) {
  Client* c = Client::mapper_[w];
  if (!c) {
    return;
  }

  if (is_fullscreen) {
    UnmapDocks();
    c->SaveXWindowAttributes();
    c->MoveResize(0, 0, wm_utils::GetDisplayResolution());
    c->workspace()->UnmapAllClients();
    c->Map();
    c->workspace()->SetFocusedClient(c->window());
  } else {
    MapDocks();
    const XWindowAttributes& attr = c->previous_attr();
    c->MoveResize(attr.x, attr.y, attr.width, attr.height);
    c->workspace()->MapAllClients();
    c->workspace()->Arrange(CalculateTilingArea());
    c->workspace()->RaiseAllFloatingClients();
    RaiseNotifications();
  }

  c->SetBorderWidth((is_fullscreen) ? 0 : config_->border_width());
  c->set_fullscreen(is_fullscreen);
  c->workspace()->set_fullscreen(is_fullscreen);

  // Update window's _NET_WM_STATE_FULLSCREEN property.
  // If the window is set to be NOT fullscreen, we will simply write a nullptr with 0 elements.
  Atom* atom = (is_fullscreen) ? &prop_->net[atom::NET_WM_STATE_FULLSCREEN] : nullptr;
  XChangeProperty(dpy_, w, prop_->net[atom::NET_WM_STATE], XA_ATOM, 32,
      PropModeReplace, (unsigned char*) atom, (is_fullscreen) ? 1 : 0);
}

void WindowManager::KillClient(Window w) {
  Atom* supported_protocols;
  int num_supported_protocols;

  // First try to kill the client gracefully via ICCCM. If the client does not support
  // this method, then we'll perform the brutal XKillClient().
  if (XGetWMProtocols(dpy_, w, &supported_protocols, &num_supported_protocols) 
      && (std::find(supported_protocols, supported_protocols + num_supported_protocols, 
          prop_->wm[atom::WM_DELETE]) != supported_protocols + num_supported_protocols)) {
    XEvent msg;
    memset(&msg, 0, sizeof(msg));
    msg.xclient.type = ClientMessage;
    msg.xclient.message_type = prop_->wm[atom::WM_PROTOCOLS];
    msg.xclient.window = w;
    msg.xclient.format = 32;
    msg.xclient.data.l[0] = prop_->wm[atom::WM_DELETE];
    XSendEvent(dpy_, w, false, 0, &msg);
  } else {
    XKillClient(dpy_, w);
  }
}


inline void WindowManager::MapDocks() const {
  for (auto w : docks_) {
    XMapWindow(dpy_, w);
  }
}

inline void WindowManager::UnmapDocks() const {
  for (auto w : docks_) {
    XUnmapWindow(dpy_, w);
  }
}

inline void WindowManager::RaiseNotifications() const {
  for (auto w : notifications_) {
    XRaiseWindow(dpy_, w);
  }
}


Area WindowManager::CalculateTilingArea() {
  pair<int, int> res = wm_utils::GetDisplayResolution();
  Area tiling_area = { 0, 0, res.first, res.second };

  for (auto w : docks_) {
    XWindowAttributes dock_attr = wm_utils::GetXWindowAttributes(w);

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

void WindowManager::DetermineFloatingWindowArea(Window w) {
  XSizeHints hints = wm_utils::GetWmNormalHints(w);
  const Area& cookie_area = cookie_->Get(w);

  // Set window size. (Priority: cookie > hints)
  if (cookie_area.width > 0 && cookie_area.height > 0) {
    XResizeWindow(dpy_, w, cookie_area.width, cookie_area.height);
  } else if (hints.min_width > 0 && hints.min_height > 0) {
    XResizeWindow(dpy_, w, hints.min_width, hints.min_height);
  } else if (hints.base_width > 0 && hints.base_height > 0) {
    XResizeWindow(dpy_, w, hints.base_width, hints.base_height);
  } else if (hints.width > 0 && hints.height > 0) {
    XResizeWindow(dpy_, w, hints.width, hints.height);
  } else {
    XResizeWindow(dpy_, w, DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
  }

  // Set window position. (Priority: cookie > hints)
  if (cookie_area.x > 0 && cookie_area.y > 0) {
    XMoveWindow(dpy_, w, cookie_area.x, cookie_area.y);
  } else if (hints.x > 0 && hints.y > 0) {
    XMoveWindow(dpy_, w, hints.x, hints.y);
  } else {
    Center(w);
  }
}


void WindowManager::UpdateClientList() {
  XDeleteProperty(dpy_, root_window_, prop_->net[atom::NET_CLIENT_LIST]);

  for (auto w : workspaces_) {
    for (auto c : w->GetClients()) {
      Window w = c->window();
      XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CLIENT_LIST], XA_WINDOW, 32, 
          PropModeAppend, (unsigned char *) &w, 1);
    }
  }
}

} // namespace wmderland
