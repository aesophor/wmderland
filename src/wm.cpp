#include "wm.hpp"
extern "C" {
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
}
#include <glog/logging.h>
#include <string>
#include <sstream>
#include <algorithm>
#include "client.hpp"
#include "util.hpp"

using std::hex;
using std::find;
using std::stoi;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;
using std::unordered_map;
using tiling::Direction;

WindowManager* WindowManager::instance_;

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
      config_(Config::GetInstance()),
      cookie_(new Cookie(COOKIE_FILE)),
      display_resolution_(wm_utils::GetDisplayResolution(dpy_, root_window_)),
      tiling_area_(Area(0, 0, display_resolution_.first, display_resolution_.second)),
      current_(0) {
    InitWorkspaces(WORKSPACE_COUNT);
    InitProperties();
    InitXEvents();
    InitCursors();
}

WindowManager::~WindowManager() {
    for (auto const w : workspaces_) {
        delete w;
    }
    
    delete prop_;
    delete config_;
    delete cookie_;
    XCloseDisplay(dpy_);
}


void WindowManager::InitWorkspaces(short count) {
    for (short i = 0; i < count; i++) {
        workspaces_[i] = new Workspace(dpy_, root_window_, i);
    }
}

void WindowManager::InitProperties() {
    // Set the name of window manager (i.e., Wmderland) on the root_window_ window,
    // so that other programs can acknowledge the name of this WM.
    XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_WM_NAME], prop_->utf8string, 8,
            PropModeReplace, (unsigned char*) WIN_MGR_NAME, sizeof(WIN_MGR_NAME));

    // Set NET_SUPPORTED to support polybar's xwindow module. However, whenever window
    // focus changes, we still have to set _NET_WM_ACTIVE_WINDOW manually!
    XChangeProperty(dpy_, root_window_, prop_->net[atom::NET_SUPPORTED], XA_ATOM, 32,
            PropModeReplace, (unsigned char*) prop_->net, atom::NET_ATOM_SIZE);
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

    // Define the key combinations to goto a specific workspace,
    // as well as moving an application to a specific workspace.
    for (int i = 0; i < 9; i++) {
        int keycode = wm_utils::StrToKeycode(dpy_, std::to_string(i + 1).c_str());
        XGrabKey(dpy_, keycode, Mod4Mask, root_window_, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy_, keycode, Mod4Mask | ShiftMask, root_window_, True, GrabModeAsync, GrabModeAsync);
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

bool WindowManager::HasResolutionChanged() {
    pair<int, int> res = wm_utils::GetDisplayResolution(dpy_, root_window_);
    return res.first != display_resolution_.first || res.second != display_resolution_.second;
}

void WindowManager::UpdateResolution() {
    pair<int, int> res = wm_utils::GetDisplayResolution(dpy_, root_window_);
    display_resolution_ = std::make_pair(res.first, res.second);
}

void WindowManager::UpdateTilingArea() {
    tiling_area_ = Area(0, 0, display_resolution_.first, display_resolution_.second);

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


void WindowManager::Run() {
    // Autostart applications specified in config.
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

void WindowManager::OnMapNotify(const XMapEvent& e) {
    Window w = e.window;

    if (IsNotification(w)
            && find(notifications_.begin(), notifications_.end(), w) == notifications_.end()) {
        notifications_.push_back(e.window);
    } 
}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    Window w = e.window;

    XClassHint class_hint = wm_utils::GetWmClass(dpy_, w);
    string res_class = string(class_hint.res_class);
    string res_name = string(class_hint.res_name);
    string wm_name = wm_utils::GetWmName(dpy_, w);
 
    // KDE Plasma Integration.
    // Make this a rule in config later.
    if (res_class == "plasmashell") {
        XKillClient(dpy_, w);
        return;
    }

    // Just map the window now. We'll discuss other things later.
    XMapWindow(dpy_, w);

    if (IsDock(w)) {
        if (find(docks_and_bars_.begin(), docks_and_bars_.end(), w) == docks_and_bars_.end()) {
            docks_and_bars_.push_back(w);
        }
        
        if (HasResolutionChanged()) {
            UpdateResolution();
            UpdateTilingArea();
        }
        return;
    }

    // If this window is a dialog, resize it to floating window width / height and center it.
    bool should_float = IsDialog(w);

    // Apply application spawning rules (if exists).
    if (config_->spawn_rules().find(res_class + ',' + res_name) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules().at(res_class + ',' + res_name) - 1;
        GotoWorkspace(target_workspace_id);
    } else if (config_->spawn_rules().find(res_class) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules().at(res_class) - 1;
        GotoWorkspace(target_workspace_id);
    }

    // Apply application floating rules (if exists).
    if (config_->float_rules().find(res_class + ',' + res_name) != config_->float_rules().end()) {
        should_float = config_->float_rules().at(res_class + ',' + res_name);
    } else if (config_->float_rules().find(res_class) != config_->float_rules().end()) {
        should_float = config_->float_rules().at(res_class);
    }


    Area stored_attr = cookie_->Get(res_class + ',' + res_name + ',' + wm_name);
    XSizeHints hint = wm_utils::GetWmNormalHints(dpy_, w);

    if (should_float) {
        if (stored_attr.width > 0 && stored_attr.height > 0) {
            XResizeWindow(dpy_, w, stored_attr.width, stored_attr.height);
        } else if (hint.min_width > 0 && hint.min_height > 0) {
            XResizeWindow(dpy_, w, hint.min_width, hint.min_height);
        } else if (hint.base_width > 0 && hint.base_height > 0) {
            XResizeWindow(dpy_, w, hint.base_width, hint.base_height);
        } else if (hint.width > 0 && hint.height > 0) {
            XResizeWindow(dpy_, w, hint.width, hint.height);
        } else {
            XResizeWindow(dpy_, w, DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
        }

        if (stored_attr.x > 0 && stored_attr.y > 0) {
            XMoveWindow(dpy_, w, stored_attr.x, stored_attr.y);
        } else if (hint.x > 0 && hint.y > 0) {
            XMoveWindow(dpy_, w, hint.x, hint.y);
        } else {
            Center(w);
        }
    }
    
    // Regular applications should be added to workspace client list,
    // but first we have to check if it's already in the list!
    if (!workspaces_[current_]->Has(w)) { 
        workspaces_[current_]->UnsetFocusedClient();
        workspaces_[current_]->Add(w, should_float);
        workspaces_[current_]->SetFocusedClient(w);
        Tile(workspaces_[current_]);
        UpdateWindowWmState(w, 1);
        SetNetActiveWindow(w);
    }

    pair<short, short> resolution = wm_utils::GetDisplayResolution(dpy_, root_window_);
    bool should_fullscreen = IsFullscreen(w);
    Client* c = Client::mapper_[w];
    if (c && ((should_fullscreen && !c->is_fullscreen()) || (hint.min_width == resolution.first && hint.min_height == resolution.second))) {
        ToggleFullscreen(w);
    }

    RaiseAllNotificationWindows();
}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {
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
    if (new_focused_client) {
        c->workspace()->SetFocusedClient(new_focused_client->window());
        SetNetActiveWindow(new_focused_client->window());
    }

    Tile(c->workspace());
    c->workspace()->RaiseAllFloatingClients();
}

void WindowManager::OnKeyPress(const XKeyEvent& e) {
    Client* focused_client = workspaces_[current_]->GetFocusedClient();

    const vector<Action*>& actions = config_->GetKeybindActions(
            wm_utils::KeymaskToStr(e.state),
            wm_utils::KeysymToStr(dpy_, e.keycode)
    );

    for (auto action : actions) {
        switch (action->type()) {
            case ActionType::TILE_H:
                workspaces_[current_]->SetTilingDirection(Direction::HORIZONTAL);
                break;
            case ActionType::TILE_V:
                workspaces_[current_]->SetTilingDirection(Direction::VERTICAL);
                break;
            case ActionType::FOCUS_LEFT:
                if (!focused_client) continue;
                workspaces_[current_]->FocusLeft();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_RIGHT:
                if (!focused_client) continue;
                workspaces_[current_]->FocusRight();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_UP:
                if (!focused_client) continue;
                workspaces_[current_]->FocusUp();
                SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
                break;
            case ActionType::FOCUS_DOWN:
                if (!focused_client) continue;
                workspaces_[current_]->FocusDown();
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
                GotoWorkspace(stoi(action->arguments()) - 1);
                break;
            case ActionType::MOVE_APP_TO_WORKSPACE:
                if (!focused_client) continue;
                MoveWindowToWorkspace(focused_client->window(), stoi(action->arguments()) - 1);
                break;
            case ActionType::KILL:
                if (!focused_client) continue;
                KillClient(focused_client->window());
                break;
            case ActionType::EXIT:
                system("pkill X");
                break;
            case ActionType::EXEC:
                system((action->arguments() + '&').c_str());
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
        // Lookup the attributes (e.g., size and position) of a window
        // and store the result in attr_
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

    // Make sure floating clients are at the top.
    //workspaces_[current_]->RaiseAllFloatingClients();
}

void WindowManager::ToggleFloating(Window w) {
    Client* c = workspaces_[current_]->GetFocusedClient();
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
