#include "wm.hpp"
#include "client.hpp"
#include "util.hpp"
extern "C" {
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
}
#include <glog/logging.h>
#include <memory>
#include <string>
#include <sstream>
#include <algorithm>

using std::hex;
using std::find;
using std::stoi;
using std::pair;
using std::string;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::stringstream;
using std::unordered_map;
using tiling::Direction;

WindowManager* WindowManager::instance_;

unique_ptr<WindowManager> WindowManager::GetInstance() {
    // If the instance is not yet initialized, we'll try to open a display
    // to X server. If it fails (i.e., dpy is None), then we return nullptr
    // to the caller. Otherwise we return an instance of WindowManager.
    if (!instance_) {
        Display* dpy;
        instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : nullptr;
    }
    return unique_ptr<WindowManager>(instance_);
}

WindowManager::WindowManager(Display* dpy) 
    : dpy_(dpy), 
      root_window_(DefaultRootWindow(dpy_)),
      prop_(make_unique<Properties>(dpy_)),
      config_(make_unique<Config>(CONFIG_FILE)),
      cookie_(make_unique<Cookie>(COOKIE_FILE)),
      display_resolution_(wm_utils::GetDisplayResolution(dpy_, root_window_)),
      tiling_area_(Area(0, 0, display_resolution_.first, display_resolution_.second)),
      current_(0) {
    InitWorkspaces(WORKSPACE_COUNT);
    InitProperties();
    InitXEvents();
    InitCursors();
}

WindowManager::~WindowManager() {
    XCloseDisplay(dpy_);
}


void WindowManager::InitWorkspaces(int count) {
    for (int i = 0; i < count; i++) {
        workspaces_[i] = new Workspace(dpy_, root_window_, config_.get(), i);
    }
}

void WindowManager::InitProperties() {
    // Set the name of window manager (i.e., Wmderland) on the root_window_ window,
    // so that other programs can acknowledge the name of this WM.
    XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_WM_NAME], prop_->utf8string, 8,
            PropModeReplace, (unsigned char*) WIN_MGR_NAME, sizeof(WIN_MGR_NAME));

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

    const char* names[WORKSPACE_COUNT] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    XTextProperty text_prop;
    Xutf8TextListToTextProperty(dpy_, (char**) names, WORKSPACE_COUNT, XUTF8StringStyle, &text_prop);
    XSetTextProperty(dpy_, root_window_, &text_prop, prop_->net[atom::NET_DESKTOP_NAMES]);
}

void WindowManager::InitXEvents() {
    // Define the key combinations which will send us X events based on the key combinations 
    // defined in config.
    for (auto r : config_->keybind_rules()) {
        vector<string> modifier_and_key = string_utils::Split(r.first, '+');
        bool shift = string_utils::Contains(r.first, "Shift");

        string modifier = modifier_and_key[0];
        string key = modifier_and_key[(shift) ? 2 : 1];

        int keycode = wm_utils::StrToKeycode(dpy_, key);
        int mod_mask = wm_utils::StrToKeymask(modifier, shift); 
        XGrabKey(dpy_, keycode, mod_mask, root_window_, True, GrabModeAsync, GrabModeAsync);
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
    cursors_[LEFT_PTR_CURSOR] = XCreateFontCursor(dpy_, XC_left_ptr);
    cursors_[RESIZE_CURSOR] = XCreateFontCursor(dpy_, XC_sizing);
    cursors_[MOVE_CURSOR] = XCreateFontCursor(dpy_, XC_fleur);
    XDefineCursor(dpy_, root_window_, cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::Run() {
    // Automatically start the applications specified in config in background.
    for (auto s : config_->autostart_rules()) {
        system((s + '&').c_str());
    }

    XEvent event;
    for (;;) {
        // Retrieve and dispatch next X event.
        XNextEvent(dpy_, &event);

        switch (event.type) {
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
            default:
                break;
        }
    }
}


void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    if (HasResolutionChanged()) {
        UpdateTilingArea();
    }

    XClassHint class_hint = wm_utils::GetWmClass(dpy_, e.window);

    // If the config says that this window should be prohibited, then
    // we should kill this window and return immediately.
    if (config_->ShouldProhibit(class_hint)) {
        XKillClient(dpy_, e.window);
        return;
    }
    
    // If this window is a dock, then add it to the docks_and_bars_ vector,
    // map it and return immediately.
    if (IsDock(e.window)
            && find(docks_and_bars_.begin(), docks_and_bars_.end(), e.window) == docks_and_bars_.end()) {
            docks_and_bars_.push_back(e.window);
            UpdateTilingArea();
            Tile(workspaces_[current_]);
            XMapWindow(dpy_, e.window);
            return;
    }

    // If everything is fine, then it means we should manage this window.
    // Float this window if it's a dialog or if such rule exists.
    bool should_float = IsDialog(e.window) || IsSplash(e.window) || config_->ShouldFloat(class_hint);

    // Spawn this window at the specified workspace if such rule exists,
    // otherwise spawn it in current workspace.
    int target_workspace_id = config_->GetSpawnWorkspaceId(class_hint);
    if (target_workspace_id == WORKSPACE_ID_NULL) target_workspace_id = current_;

    // Add this window to the target workspace.
    if (!workspaces_[target_workspace_id]->Has(e.window)) {
        workspaces_[target_workspace_id]->UnsetFocusedClient();
        workspaces_[target_workspace_id]->Add(e.window, should_float);
        Tile(workspaces_[target_workspace_id]);
        UpdateWindowWmState(e.window, 1);

        if (target_workspace_id == current_) {
            XMapWindow(dpy_, e.window);
            SetNetActiveWindow(e.window);
            workspaces_[target_workspace_id]->SetFocusedClient(e.window);
        }
    }
    
    if (target_workspace_id == current_) {
        Area cookie_area = cookie_->Get(class_hint, wm_utils::GetWmName(dpy_, e.window));
        XSizeHints hints = wm_utils::GetWmNormalHints(dpy_, e.window);

        if (should_float) {
            RestoreWindowPosSize(e.window, cookie_area, hints);
        }

        pair<short, short> resolution = wm_utils::GetDisplayResolution(dpy_, root_window_);
        bool should_fullscreen = IsFullscreen(e.window);
        Client* c = Client::mapper_[e.window];
        if (c && ((should_fullscreen && !c->is_fullscreen()) || (hints.min_width == resolution.first && hints.min_height == resolution.second))) {
            ToggleFullscreen(e.window);
        }
    }

    RaiseAllNotificationWindows();
}

void WindowManager::OnMapNotify(const XMapEvent& e) {
    Window w = e.window;

    if (IsNotification(w)
            && find(notifications_.begin(), notifications_.end(), w) == notifications_.end()) {
        notifications_.push_back(e.window);
    } 
}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {
    if (find(docks_and_bars_.begin(), docks_and_bars_.end(), e.window) != docks_and_bars_.end()) {
        docks_and_bars_.erase(remove(docks_and_bars_.begin(), docks_and_bars_.end(), e.window), docks_and_bars_.end());
        UpdateTilingArea();
        Tile(workspaces_[current_]);
    }

    // If we aren't managing this window, return at once.
    if (!Client::mapper_[e.window]) return;
    Client* c = Client::mapper_[e.window];

    // If the client being destroyed is in fullscreen mode, make sure to
    // unset the workspace's fullscreen state.
    if (c->is_fullscreen()) {
        c->workspace()->set_fullscreen(false);
        c->workspace()->MapAllClients();
        MapDocksAndBars();
    }

    if (IsNotification(e.window)) {
        notifications_.erase(remove(notifications_.begin(), notifications_.end(), e.window), notifications_.end());
    }

    // Remove the corresponding client from the client tree.
    c->workspace()->Remove(e.window);
    UpdateWindowWmState(e.window, 0);
    ClearNetActiveWindow();

    // Transfer focus to another window (if there's still one).
    Client* new_focused_client = c->workspace()->GetFocusedClient();
    if (c->workspace() == workspaces_[current_] && new_focused_client) {
        c->workspace()->SetFocusedClient(new_focused_client->window());
        SetNetActiveWindow(new_focused_client->window());
    }

    Tile(c->workspace());
    c->workspace()->RaiseAllFloatingClients();
}

void WindowManager::OnKeyPress(const XKeyEvent& e) {
    Client* focused_client = workspaces_[current_]->GetFocusedClient();

    const vector<Action>& actions = config_->GetKeybindActions(
            wm_utils::KeymaskToStr(e.state), // modifier str, e.g., "Mod4+Shift"
            wm_utils::KeysymToStr(dpy_, e.keycode) // key str, e.g., "q"
    );

    for (auto action : actions) { 
        switch (action.type()) {
            case ActionType::TILE_H:
                workspaces_[current_]->SetTilingDirection(Direction::HORIZONTAL);
                break;
            case ActionType::TILE_V:
                workspaces_[current_]->SetTilingDirection(Direction::VERTICAL);
                break;
            case ActionType::FOCUS_LEFT:
                if (!focused_client || workspaces_[current_]->is_fullscreen()) continue;
                workspaces_[current_]->FocusLeft();
                workspaces_[current_]->RaiseAllFloatingClients();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_RIGHT:
                if (!focused_client || workspaces_[current_]->is_fullscreen()) continue;
                workspaces_[current_]->FocusRight();
                workspaces_[current_]->RaiseAllFloatingClients();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_UP:
                if (!focused_client || workspaces_[current_]->is_fullscreen()) continue;
                workspaces_[current_]->FocusUp();
                workspaces_[current_]->RaiseAllFloatingClients();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_DOWN:
                if (!focused_client || workspaces_[current_]->is_fullscreen()) continue;
                workspaces_[current_]->FocusDown();
                workspaces_[current_]->RaiseAllFloatingClients();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::TOGGLE_FLOATING:
                if (!focused_client) continue;
                ToggleFloating(focused_client->window());
                break;
            case ActionType::TOGGLE_FULLSCREEN:
                if (!focused_client) continue;
                ToggleFullscreen(focused_client->window());
                break;
            case ActionType::GOTO_WORKSPACE:
                GotoWorkspace(stoi(action.arguments()) - 1);
                break;
            case ActionType::MOVE_APP_TO_WORKSPACE:
                if (!focused_client) continue;
                MoveWindowToWorkspace(focused_client->window(), stoi(action.arguments()) - 1);
                break;
            case ActionType::KILL:
                if (!focused_client) continue;
                KillClient(focused_client->window());
                break;
            case ActionType::EXIT:
                system("pkill X");
                break;
            case ActionType::EXEC:
                system((action.arguments() + '&').c_str());
                break;
            default:
                break;
        }
    }

    RaiseAllNotificationWindows();
}

void WindowManager::OnButtonPress(const XButtonEvent& e) {
    if (!e.subwindow) return;

    Client* c = Client::mapper_[e.subwindow];
    if (!c || !c->is_floating()) {
        return;
    } 

    if (e.state == Mod4Mask) {
        XGetWindowAttributes(dpy_, e.subwindow, &(c->previous_attr()));
        btn_pressed_event_ = e;

        XDefineCursor(dpy_, root_window_, cursors_[e.button]);
    }
}

void WindowManager::OnButtonRelease(const XButtonEvent& e) {
    if (!btn_pressed_event_.subwindow) return;

    XWindowAttributes attr = wm_utils::GetWindowAttributes(dpy_, btn_pressed_event_.subwindow);
    XClassHint class_hint = wm_utils::GetWmClass(dpy_, btn_pressed_event_.subwindow);
    string res_class_name = string(class_hint.res_class) + ',' + string(class_hint.res_name);
    string wm_name = wm_utils::GetWmName(dpy_, btn_pressed_event_.subwindow);
    cookie_->Put(res_class_name + ',' + wm_name, Area(attr.x, attr.y, attr.width, attr.height));

    btn_pressed_event_.subwindow = None;
    XDefineCursor(dpy_, root_window_, cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::OnMotionNotify(const XButtonEvent& e) {
    if (!btn_pressed_event_.subwindow) return;

    Client* c = Client::mapper_[btn_pressed_event_.subwindow];
    if (!c) return;
    XWindowAttributes& attr = c->previous_attr();

    int xdiff = e.x - btn_pressed_event_.x;
    int ydiff = e.y - btn_pressed_event_.y;
    int new_x = attr.x + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? xdiff : 0);
    int new_y = attr.y + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? ydiff : 0);
    int new_width = attr.width + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? xdiff : 0);
    int new_height = attr.height + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? ydiff : 0);

    new_width = (new_width < MIN_WINDOW_WIDTH) ? MIN_WINDOW_WIDTH : new_width;
    new_height = (new_height < MIN_WINDOW_HEIGHT) ? MIN_WINDOW_HEIGHT : new_height;
    XMoveResizeWindow(dpy_, btn_pressed_event_.subwindow, new_x, new_y, new_width, new_height);
}

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
    const int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(dpy, e->error_code, error_text, sizeof(error_text));
    LOG(ERROR) << "Received X error:\n"
        << "    Request: " << int(e->request_code)
        << "    Error code: " << int(e->error_code)
        << " - " << error_text << "\n"
        << "    Resource ID: " << e->resourceid;
    // The return value is ignored.
    return 0;
}


bool WindowManager::IsDock(Window w) {
    Atom property = prop_->net[atom::NET_WM_WINDOW_TYPE];
    Atom atom = prop_->net[atom::NET_WM_WINDOW_TYPE_DOCK];
    return wm_utils::WindowPropertyHasAtom(dpy_, w, property, atom);
}

bool WindowManager::IsDialog(Window w) {
    Atom property = prop_->net[atom::NET_WM_WINDOW_TYPE];
    Atom atom = prop_->net[atom::NET_WM_WINDOW_TYPE_DIALOG];
    return wm_utils::WindowPropertyHasAtom(dpy_, w, property, atom);
}

bool WindowManager::IsSplash(Window w) {
    Atom property = prop_->net[atom::NET_WM_WINDOW_TYPE];
    Atom atom = prop_->net[atom::NET_WM_WINDOW_TYPE_SPLASH];
    return wm_utils::WindowPropertyHasAtom(dpy_, w, property, atom);
}

bool WindowManager::IsNotification(Window w) {
    Atom property = prop_->net[atom::NET_WM_WINDOW_TYPE];
    Atom atom = prop_->net[atom::NET_WM_WINDOW_TYPE_NOTIFICATION];
    return wm_utils::WindowPropertyHasAtom(dpy_, w, property, atom);
}

bool WindowManager::IsFullscreen(Window w) {
    Atom property = prop_->net[atom::NET_WM_STATE];
    Atom atom = prop_->net[atom::NET_WM_STATE_FULLSCREEN];
    return wm_utils::WindowPropertyHasAtom(dpy_, w, property, atom);
}


bool WindowManager::HasResolutionChanged() {
    pair<int, int> res = wm_utils::GetDisplayResolution(dpy_, root_window_);
    return res.first != display_resolution_.first || res.second != display_resolution_.second;
}

void WindowManager::UpdateTilingArea() {
    pair<int, int> res = wm_utils::GetDisplayResolution(dpy_, root_window_);
    display_resolution_ = std::make_pair(res.first, res.second);
    tiling_area_ = { 0, 0, res.first, res.second };

    for (auto w : docks_and_bars_) {
        XWindowAttributes dock_attr = wm_utils::GetWindowAttributes(dpy_, w);

        if (dock_attr.y == 0) {
            // If the dock is at the top of the screen.
            tiling_area_.y += dock_attr.height;
            tiling_area_.height -= dock_attr.height;
        } else if (dock_attr.y + dock_attr.height == tiling_area_.y + tiling_area_.height) {
            // If the dock is at the bottom of the screen.
            tiling_area_.height -= dock_attr.height;
        } else if (dock_attr.x == 0) {
            // If the dock is at the leftmost of the screen.
            tiling_area_.x += dock_attr.width;
            tiling_area_.width -= dock_attr.width;
        } else if (dock_attr.x + dock_attr.width == tiling_area_.x + tiling_area_.width) {
            // If the dock is at the rightmost of the screen.
            tiling_area_.width -= dock_attr.width;
        }
    }
}

void WindowManager::RestoreWindowPosSize(Window w, const Area& cookie_area, const XSizeHints& hints) {
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


void WindowManager::MapDocksAndBars() {
    for (auto w : docks_and_bars_) {
        XMapWindow(dpy_, w);
    }
}

void WindowManager::UnmapDocksAndBars() {
    for (auto w : docks_and_bars_) {
        XUnmapWindow(dpy_, w);
    }
}

void WindowManager::RaiseAllNotificationWindows() {
    for (auto w : notifications_) {
        XRaiseWindow(dpy_, w);
    }
}


void WindowManager::UpdateWindowWmState(Window w, unsigned long state) {
    // Set full WM_STATE according to
    // http://www.x.org/releases/X11R7.7/doc/xorg-docs/icccm/icccm.html#WM_STATE_Property
    // WithdrawnState: 0
    // NormalState: 1
    // IconicState: 3
    unsigned long wmstate[] = { state, None };
    XChangeProperty(dpy_, w,  prop_->wm[atom::WM_STATE], prop_->wm[atom::WM_STATE], 32, 
            PropModeReplace, (unsigned char*) wmstate, 2);
}

void WindowManager::SetNetActiveWindow(Window w) {
    Client* c = workspaces_[current_]->GetClient(w);
    XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_ACTIVE_WINDOW], XA_WINDOW, 32,
            PropModeReplace, (unsigned char*) &(c->window()), 1);
}

void WindowManager::ClearNetActiveWindow() {
    XDeleteProperty(dpy_, root_window_, prop_->net[atom::NET_ACTIVE_WINDOW]);
}


void WindowManager::GotoWorkspace(int next) {
    if (current_ == next) return;

    MapDocksAndBars();
    ClearNetActiveWindow();

    workspaces_[current_]->UnmapAllClients();
    workspaces_[next]->MapAllClients();
    current_ = next;

    Client* focused_client = workspaces_[current_]->GetFocusedClient();
    if (focused_client) {
        workspaces_[current_]->SetFocusedClient(focused_client->window());
        SetNetActiveWindow(focused_client->window());
        Tile(workspaces_[current_]);

        // Restore fullscreen application.
        if (focused_client->is_fullscreen()) {
            int screen_width = display_resolution_.first;
            int screen_height = display_resolution_.second;
            XMoveResizeWindow(dpy_, focused_client->window(), 0, 0, screen_width, screen_height);
            UnmapDocksAndBars();
        }
    }

    unsigned long current_workspace = current_;
    XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_CURRENT_DESKTOP], XA_CARDINAL, 32, 
            PropModeReplace, (unsigned char *) &current_workspace, 1);
}

void WindowManager::MoveWindowToWorkspace(Window window, int next) {    
    if (current_ == next) return;

    Client* c = Client::mapper_[window];
    if (!c) return;

    if (c->is_fullscreen()) {
        workspaces_[current_]->set_fullscreen(false);
        c->set_fullscreen(false);
        c->workspace()->MapAllClients();
        MapDocksAndBars();
    }

    XUnmapWindow(dpy_, window);

    workspaces_[next]->UnsetFocusedClient();
    workspaces_[current_]->Move(window, workspaces_[next]);
    Client* focused_client = workspaces_[current_]->GetFocusedClient();

    if (!focused_client) {
        ClearNetActiveWindow();
        return;
    }

    workspaces_[current_]->SetFocusedClient(focused_client->window());
    workspaces_[next]->SetFocusedClient(window);
    Tile(workspaces_[current_]);
}


void WindowManager::Center(Window w) {
    pair<short, short> display_resolution = wm_utils::GetDisplayResolution(dpy_, root_window_);
    short screen_width = display_resolution.first;
    short screen_height = display_resolution.second;

    XWindowAttributes w_attr = wm_utils::GetWindowAttributes(dpy_, w);
    int new_x = screen_width / 2 - w_attr.width / 2;
    int new_y = screen_height / 2 - w_attr.height / 2;
    XMoveWindow(dpy_, w, new_x, new_y);
}

void WindowManager::Tile(Workspace* workspace) {
    UpdateTilingArea();
    workspace->Arrange(tiling_area_);
}

void WindowManager::ToggleFloating(Window w) {
    Client* c = Client::mapper_[w];
    if (!c) return;

    // TODO: Fix floating and fullscreen.
    c->set_floating(!c->is_floating());
    if (c->is_floating()) {
        XResizeWindow(dpy_, c->window(), DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
        Center(c->window());
    }
    Tile(workspaces_[current_]);
}

void WindowManager::ToggleFullscreen(Window w) {
    Client* c = Client::mapper_[w];
    if (!c) return;

    XWindowAttributes& attr = c->previous_attr();

    if (!workspaces_[current_]->is_fullscreen() && !c->is_fullscreen()) {
        pair<short, short> display_resolution = wm_utils::GetDisplayResolution(dpy_, root_window_);
        short screen_width = display_resolution.first;
        short screen_height = display_resolution.second;

        // Record the current window's position and size before making it fullscreen.
        XGetWindowAttributes(dpy_, w, &attr);
        XChangeProperty(dpy_, w, prop_->net[atom::NET_WM_STATE], XA_ATOM, 32,
                PropModeReplace, (unsigned char*) &prop_->net[atom::NET_WM_STATE_FULLSCREEN], 1);
        XMoveResizeWindow(dpy_, w, 0, 0, screen_width, screen_height);
        c->SetBorderWidth(0);

        workspaces_[current_]->set_fullscreen(true);
        c->set_fullscreen(true);

        workspaces_[current_]->UnmapAllClients();
        UnmapDocksAndBars();
        c->Map();
        c->SetInputFocus();
    } else if (workspaces_[current_]->is_fullscreen() && c->is_fullscreen()) {
        XChangeProperty(dpy_, w, prop_->net[atom::NET_WM_STATE], XA_ATOM, 32,
                PropModeReplace, (unsigned char*) 0, 0);
        // Restore the window to its original position and size.
        XMoveResizeWindow(dpy_, w, attr.x, attr.y, attr.width, attr.height);
        c->SetBorderWidth(config_->border_width());

        workspaces_[current_]->set_fullscreen(false);
        c->set_fullscreen(false);

        workspaces_[current_]->MapAllClients();
        MapDocksAndBars();
    }

    XRaiseWindow(dpy_, w);
}

void WindowManager::KillClient(Window w) {
    Atom* supported_protocols;
    int num_supported_protocols;

    // First try to kill the client gracefully via ICCCM.  If the client does not support
    // this method, then we'll perform the brutal XKillClient().
    if (XGetWMProtocols(dpy_, w, &supported_protocols, &num_supported_protocols) 
            && (find(supported_protocols, supported_protocols + num_supported_protocols, 
                    prop_->wm[atom::WM_DELETE]) != supported_protocols + num_supported_protocols)) {
        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = prop_->wm[atom::WM_PROTOCOLS];
        msg.xclient.window = w;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = prop_->wm[atom::WM_DELETE];
        CHECK(XSendEvent(dpy_, w, false, 0, &msg));
    } else {
        XKillClient(dpy_, w);
    }
}
