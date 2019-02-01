extern "C" {
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
}
#include <glog/logging.h>
#include <string>
#include <sstream>
#include <algorithm>
#include "wm.hpp"
#include "client.hpp"
#include "tiling.hpp"
#include "util.hpp"

using std::hex;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;
using std::unordered_map;
using tiling::Direction;
using tiling::Action;

WindowManager* WindowManager::instance_;

WindowManager* WindowManager::GetInstance() {
    // If the instance is not yet initialized, we'll try to open a display
    // to X server. If it fails (i.e., dpy is None), then we return None
    // to the caller. Otherwise we return an instance of WindowManager.
    if (!instance_) {
        Display* dpy;
        instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : nullptr;
    }
    return instance_;
}

WindowManager::WindowManager(Display* dpy) : dpy_(dpy), root_(DefaultRootWindow(dpy_)) {
    current_ = 0;
    prop_ = new Properties(dpy_);
    config_ = Config::GetInstance();
    cookie_ = new Cookie(COOKIE_FILE);

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
        workspaces_[i] = new Workspace(dpy_, root_, i);
    }
}

void WindowManager::InitProperties() {
    // Set the name of window manager (i.e., Wmderland) on the root_ window,
    // so that other programs can acknowledge the name of this WM.
    XChangeProperty(dpy_, root_, prop_->net_atoms[atom::NET_WM_NAME], prop_->utf8string, 8,
            PropModeReplace, (unsigned char*) WIN_MGR_NAME, sizeof(WIN_MGR_NAME));

    // Set NET_SUPPORTED to support polybar's xwindow module.
    // However, we still have to set _NET_WM_ACTIVE_WINDOW manually!
    XChangeProperty(dpy_, root_, prop_->net_atoms[atom::NET_SUPPORTED], XA_ATOM, 32,
            PropModeReplace, (unsigned char*) prop_->net_atoms, atom::NET_ATOM_SIZE);
}

void WindowManager::InitXEvents() {
    // Define the key combinations which will send us X events based on
    // the key combinations that user has defined in config.
    for (auto r : config_->keybind_rules()) {
        vector<string> modifier_and_key = string_utils::Split(r.first, '+');
        bool shift = string_utils::Contains(r.first, "Shift");

        string modifier = modifier_and_key[0];
        string key = modifier_and_key[(shift) ? 2 : 1];

        int keycode = wm_utils::QueryKeycode(dpy_, key);
        int mod_mask = wm_utils::StrToKeymask(modifier, shift); 
        XGrabKey(dpy_, keycode, mod_mask, root_, True, GrabModeAsync, GrabModeAsync);
    }

    // Define the key combinations to goto a specific workspace,
    // as well as moving an application to a specific workspace.
    for (int i = 0; i < 9; i++) {
        int keycode = wm_utils::QueryKeycode(dpy_, std::to_string(i+1).c_str());
        XGrabKey(dpy_, keycode, Mod4Mask, root_, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(dpy_, keycode, Mod4Mask | ShiftMask, root_, True, GrabModeAsync, GrabModeAsync);
    }

    // Define which mouse clicks will send us X events.
    XGrabButton(dpy_, AnyButton, Mod4Mask, root_, True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    // Enable substructure redirection on the root window.
    XSelectInput(dpy_, root_, SubstructureNotifyMask | SubstructureRedirectMask);

    // Setup the bitch catcher.
    XSetErrorHandler(&WindowManager::OnXError);
}

void WindowManager::InitCursors() {
    cursors_[LEFT_PTR_CURSOR] = XCreateFontCursor(dpy_, XC_left_ptr);
    cursors_[RESIZE_CURSOR] = XCreateFontCursor(dpy_, XC_sizing);
    cursors_[MOVE_CURSOR] = XCreateFontCursor(dpy_, XC_fleur);
    SetCursor(root_, cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::SetCursor(Window w, Cursor c) {
    XDefineCursor(dpy_, w, c);
}


void WindowManager::Run() {
    // Autostart applications specified in config.
    for (auto s : config_->autostart_rules()) {
        system(s.c_str());
    }

    for (;;) {
        // Retrieve and dispatch next X event.
        XNextEvent(dpy_, &event_);

        switch (event_.type) {
            case MapRequest:
                OnMapRequest(event_.xmaprequest.window);
                break;
            case DestroyNotify:
                OnDestroyNotify(event_.xdestroywindow.window);
                break;
            case KeyPress:
                OnKeyPress();
                break;
            case ButtonPress:
                OnButtonPress();
                break;
            case ButtonRelease:
                OnButtonRelease();
                break;
            case MotionNotify:
                OnMotionNotify();
                break;
            default:
                break;
        }
    }
}

void WindowManager::Stop() {
    system("pkill X");
}


void WindowManager::OnMapRequest(Window w) {
    XClassHint class_hint = wm_utils::QueryWmClass(dpy_, w);
    string res_class = string(class_hint.res_class);
    string res_name = string(class_hint.res_name);
    string res_class_name = res_class + ',' + res_name;
    string wm_name = wm_utils::QueryWmName(dpy_, w);

    // KDE Plasma Integration.
    // Make this a rule in config later.
    if (res_class == "plasmashell") {
        XKillClient(dpy_, w);
        return;
    }

    // Just map the window now. We'll discuss other things later.
    XMapWindow(dpy_, w);

    // Bars should not have border or be added to a workspace.
    // We check if w is a bar by inspecting its WM_CLASS.
    if (wm_utils::IsBar(dpy_, w)) {
        XWindowAttributes attr = wm_utils::QueryWindowAttributes(dpy_, w);
        bar_height_ = attr.height;
        bar_ = w;
        return;
    }
 
    // If this window is a dialog, resize it to floating window width / height and center it.
    bool should_float = wm_utils::IsDialogOrNotification(dpy_, w, prop_->net_atoms);

    // Apply application spawning rules (if exists).
    if (config_->spawn_rules().find(res_class_name) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules()[res_class_name]- 1;
        GotoWorkspace(target_workspace_id);
    } else if (config_->spawn_rules().find(res_class) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules()[res_class] - 1;
        GotoWorkspace(target_workspace_id);
    }

    // Apply application floating rules (if exists).
    if (config_->float_rules().find(res_class_name) != config_->float_rules().end()) {
        should_float = config_->float_rules()[res_class_name];
    } else if (config_->float_rules().find(res_class) != config_->float_rules().end()) {
        should_float = config_->float_rules()[res_class];
    }


    WindowPosSize stored_attr = cookie_->Get(res_class_name + ',' + wm_name);
    XSizeHints hint = wm_utils::QueryWmNormalHints(dpy_, w);

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

        // XSelectInput() and Borders are automatically done in the constructor of Client class.
        workspaces_[current_]->Add(w, should_float);
    }

    // Set the newly mapped client as the focused one.
    workspaces_[current_]->SetFocusedClient(w);
    Tile(workspaces_[current_]);
    SetNetActiveWindow(w);

    //pair<short, short> resolution = wm_utils::GetDisplayResolution(dpy_, root_);
    //bool should_fullscreen = wm_utils::IsFullScreen(dpy_, w, prop_->net_atoms);
    //Client* c = Client::mapper_[w];
    //if (c && ((should_fullscreen && !c->is_fullscreen()) || (hint.min_width == resolution.first && hint.min_height == resolution.second))) {
    //    ToggleFullScreen(w);
    //}
}

void WindowManager::OnDestroyNotify(Window w) {
    // When a window is destroyed, remove it from the current workspace's client list.    
    Client* c = Client::mapper_[w];
    if (!c) return;

    // If the client we are destroying is fullscreen,
    // make sure to unset the workspace's fullscreen state.
    if (c->is_fullscreen()) {
        c->workspace()->set_fullscreen(false);
    }

    // If the client being destroyed is within current workspace,
    // remove it from current workspace's client list.
    if (workspaces_[current_]->Has(w)) {
        workspaces_[current_]->Remove(w);
        Tile(workspaces_[current_]);
        ClearNetActiveWindow();

        // Since the previously active window has been killed, we should
        // manually set focus to another window.
        Client* new_focused_client = workspaces_[current_]->GetFocusedClient();
        if (new_focused_client) {
            workspaces_[current_]->SetFocusedClient(new_focused_client->window());
            SetNetActiveWindow(new_focused_client->window());
        }
    } else {
        c->workspace()->Remove(w);
    }

    //c->workspace()->RaiseAllFloatingClients();
}

void WindowManager::OnKeyPress() {
    Client* focused_client = workspaces_[current_]->GetFocusedClient();
    Window w = (focused_client) ? focused_client->window() : None;
 
    // Move application to a specific workspace,
    // and goto a specific workspace.
    if (event_.xkey.state == (Mod4Mask | ShiftMask)
            && event_.xkey.keycode >= XKeysymToKeycode(dpy_, XStringToKeysym("1"))
            && event_.xkey.keycode <= XKeysymToKeycode(dpy_, XStringToKeysym("9"))) {
        MoveWindowToWorkspace(w, event_.xkey.keycode - 10);
        return;
    } else if (event_.xkey.state == Mod4Mask
            && event_.xkey.keycode >= XKeysymToKeycode(dpy_, XStringToKeysym("1"))
            && event_.xkey.keycode <= XKeysymToKeycode(dpy_, XStringToKeysym("9"))) {
        GotoWorkspace(event_.xkey.keycode - 10);
        return;
    }
 
    string modifier = wm_utils::KeymaskToStr(event_.xkey.state);
    string key = wm_utils::QueryKeysym(dpy_, event_.xkey.keycode, false);
    Action action = config_->GetKeybindAction(modifier, key);

    if (action == Action::EXEC) {
        string command = config_->keybind_cmds()[modifier + '+' + key] + '&';
        system(command.c_str());
        return;
    } else if (action == Action::EXIT) {
        Stop();
    } else if (action == Action::TILE_H) {
        workspaces_[current_]->SetTilingDirection(Direction::HORIZONTAL);
    } else if (action == Action::TILE_V) {
        workspaces_[current_]->SetTilingDirection(Direction::VERTICAL);
    }

    if (w == None) return;

    switch (action) {
        case Action::FOCUS_LEFT:
            workspaces_[current_]->FocusLeft();
            SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
            break;
        case Action::FOCUS_RIGHT:
            workspaces_[current_]->FocusRight();
            SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
            break;
        case Action::FOCUS_DOWN:
            workspaces_[current_]->FocusDown();
            SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
            break;
        case Action::FOCUS_UP:
            workspaces_[current_]->FocusUp();
            SetNetActiveWindow(workspaces_[current_]->GetFocusedClient()->window());
            break;
        case Action::TOGGLE_FLOATING:
            ToggleFloating(w);
            break;
        case Action::TOGGLE_FULLSCREEN:
            ToggleFullScreen(w);
            break;
        case Action::KILL:
            KillClient(w);
            break;
        default:
            break;
    }
}

void WindowManager::OnButtonPress() {
    Window w = event_.xbutton.subwindow;
    if (w == None) return;

    Client* c = Client::mapper_[w];
    if (!c || !c->is_floating()) {
        return;
    } 

    if (event_.xbutton.state == Mod4Mask) {
        // Lookup the attributes (e.g., size and position) of a window
        // and store the result in attr_
        XGetWindowAttributes(dpy_, w, &c->previous_attr());
        start_ = event_.xbutton;

        SetCursor(root_, cursors_[event_.xbutton.button]);
    }
}

void WindowManager::OnButtonRelease() {
    if (start_.subwindow == None) return;

    XWindowAttributes attr = wm_utils::QueryWindowAttributes(dpy_, start_.subwindow);
    XClassHint class_hint = wm_utils::QueryWmClass(dpy_, start_.subwindow);
    string res_class_name = string(class_hint.res_class) + ',' + string(class_hint.res_name);
    string wm_name = wm_utils::QueryWmName(dpy_, start_.subwindow);
    cookie_->Put(res_class_name + ',' + wm_name, WindowPosSize(attr.x, attr.y, attr.width, attr.height));

    start_.subwindow = None;
    SetCursor(root_, cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::OnMotionNotify() {
    Window w = start_.subwindow;
    if (w == None) return;

    Client* c = Client::mapper_[w];
    if (!c) return;

    int xdiff = event_.xbutton.x - start_.x;
    int ydiff = event_.xbutton.y - start_.y;

    XWindowAttributes& attr = c->previous_attr();
    int new_x = attr.x + ((start_.button == MOUSE_LEFT_BTN) ? xdiff : 0);
    int new_y = attr.y + ((start_.button == MOUSE_LEFT_BTN) ? ydiff : 0);
    int new_width = attr.width + ((start_.button == MOUSE_RIGHT_BTN) ? xdiff : 0);
    int new_height = attr.height + ((start_.button == MOUSE_RIGHT_BTN) ? ydiff : 0);

    if (new_width < MIN_WINDOW_WIDTH) new_width = MIN_WINDOW_WIDTH;
    if (new_height < MIN_WINDOW_HEIGHT) new_height = MIN_WINDOW_HEIGHT;
    XMoveResizeWindow(dpy_, w, new_x, new_y, new_width, new_height);
}


void WindowManager::SetNetActiveWindow(Window focused_window) {
    Client* c = workspaces_[current_]->GetClient(focused_window);
    XChangeProperty(dpy_, root_, prop_->net_atoms[atom::NET_ACTIVE_WINDOW], XA_WINDOW, 32,
            PropModeReplace, (unsigned char*) &(c->window()), 1);
}

void WindowManager::ClearNetActiveWindow() {
    XDeleteProperty(dpy_, root_, prop_->net_atoms[atom::NET_ACTIVE_WINDOW]);
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


void WindowManager::GotoWorkspace(short next) {
    if (current_ == next) return;

    ClearNetActiveWindow();

    workspaces_[current_]->UnmapAllClients();
    workspaces_[next]->MapAllClients();
    current_ = next;

    Client* focused_client = workspaces_[current_]->GetFocusedClient();
    if (focused_client) {
        workspaces_[current_]->SetFocusedClient(focused_client->window());
        SetNetActiveWindow(focused_client->window());
        Tile(workspaces_[current_]);
    }
}

void WindowManager::MoveWindowToWorkspace(Window window, short next) {    
    if (current_ == next) return;

    Client* c = Client::mapper_[window];
    if (!c) return;

    if (c->is_fullscreen()) {
        workspaces_[current_]->set_fullscreen(false);
        c->set_fullscreen(false);
    }

    XUnmapWindow(dpy_, window);
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
    pair<short, short> display_resolution = wm_utils::GetDisplayResolution(dpy_, root_);
    short screen_width = display_resolution.first;
    short screen_height = display_resolution.second;

    XWindowAttributes w_attr = wm_utils::QueryWindowAttributes(dpy_, w);
    int new_x = screen_width / 2 - w_attr.width / 2;
    int new_y = screen_height / 2 - w_attr.height / 2;
    XMoveWindow(dpy_, w, new_x, new_y);
}

void WindowManager::Tile(Workspace* workspace) {
    workspace->Arrange(bar_height_);

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

void WindowManager::ToggleFullScreen(Window w) {
    Client* c = Client::mapper_[w];
    if (!c) return;

    XWindowAttributes& attr = c->previous_attr();

    if (!workspaces_[current_]->is_fullscreen() && !c->is_fullscreen()) {
        pair<short, short> display_resolution = wm_utils::GetDisplayResolution(dpy_, root_);
        short screen_width = display_resolution.first;
        short screen_height = display_resolution.second;

        // Record the current window's position and size before making it fullscreen.
        XGetWindowAttributes(dpy_, w, &attr);
        XChangeProperty(dpy_, w, prop_->net_atoms[atom::NET_WM_STATE], XA_ATOM, 32,
                PropModeReplace, (unsigned char*) &prop_->net_atoms[atom::NET_WM_STATE_FULLSCREEN], 1);
        XMoveResizeWindow(dpy_, w, 0, 0, screen_width, screen_height);
        c->SetBorderWidth(0);

        workspaces_[current_]->set_fullscreen(true);
        c->set_fullscreen(true);

        workspaces_[current_]->UnmapAllClients();
        XMapWindow(dpy_, w);
    } else if (workspaces_[current_]->is_fullscreen() && c->is_fullscreen()) {
        XChangeProperty(dpy_, w, prop_->net_atoms[atom::NET_WM_STATE], XA_ATOM, 32,
                PropModeReplace, (unsigned char*) 0, 0);
        // Restore the window to its original position and size.
        XMoveResizeWindow(dpy_, w, attr.x, attr.y, attr.width, attr.height);
        c->SetBorderWidth(config_->border_width());

        workspaces_[current_]->set_fullscreen(false);
        c->set_fullscreen(false);

        workspaces_[current_]->MapAllClients();
    }

    XRaiseWindow(dpy_, w);
}

void WindowManager::KillClient(Window w) {
    Atom* supported_protocols;
    int num_supported_protocols;

    // First try to kill the client gracefully via ICCCM.
    // If the client does not support this method, then
    // we'll perform the brutal XKillClient().
    if (XGetWMProtocols(dpy_, w, &supported_protocols, &num_supported_protocols) 
            && (::std::find(supported_protocols, supported_protocols + num_supported_protocols, 
                    prop_->wm_atoms[atom::WM_DELETE]) != supported_protocols + num_supported_protocols)) {
        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = prop_->wm_atoms[atom::WM_PROTOCOLS];
        msg.xclient.window = w;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = prop_->wm_atoms[atom::WM_DELETE];
        CHECK(XSendEvent(dpy_, w, false, 0, &msg));
    } else {
        XKillClient(dpy_, w);
    }
}
