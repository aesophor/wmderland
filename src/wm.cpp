#include "wm.hpp"
#include "global.hpp"
#include "client.hpp"
#include "util.hpp"
#include <string>
#include <algorithm>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <glog/logging.h>

WindowManager* WindowManager::instance_;

WindowManager* WindowManager::GetInstance() {
    // If the instance is not yet initialized, we'll try to open a display
    // to X server. If it fails (i.e., dpy is None), then we return None
    // to the caller. Otherwise we return an instance of WindowManager.
    if (!instance_) {
        Display* dpy;
        instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : None;
    }
    return instance_;
}

WindowManager::WindowManager(Display* dpy) {
    dpy_ = dpy;
    current_ = 0;
    fullscreen_ = false;
    tiling_direction_ = Direction::HORIZONTAL;
    //config_ = new Config(CONFIG_FILE);
    properties_ = new Properties(dpy_);

    InitWorkspaces(WORKSPACE_COUNT);
    InitProperties();
    InitXEvents();
    InitCursors();
}

WindowManager::~WindowManager() {
    for (auto const w : workspaces_) {
        delete w;
    }
    delete properties_;
    XCloseDisplay(dpy_);
}


void WindowManager::InitWorkspaces(short count) {
    for (short i = 0; i < count; i++) {
        workspaces_.push_back(new Workspace(dpy_, i));
    }
}

void WindowManager::InitProperties() {
    // Initialize property manager and set _NET_WM_NAME.
    properties_->Set(
            DefaultRootWindow(dpy_), 
            properties_->net_atoms_[atom::NET_WM_NAME],
            properties_->utf8string_,
            8, PropModeReplace, (unsigned char*) WM_NAME, sizeof(WM_NAME)
    );
    properties_->Set(
            DefaultRootWindow(dpy_),
            properties_->net_atoms_[atom::NET_SUPPORTED],
            XA_ATOM, 32, PropModeReplace, (unsigned char*) properties_->net_atoms_,
            atom::NET_ATOM_SIZE
    );
}

void WindowManager::InitXEvents() {
    // Define which key combinations will send us X events.
    XGrabKey(dpy_, AnyKey, Mod1Mask, DefaultRootWindow(dpy_), True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy_, AnyKey, Mod4Mask, DefaultRootWindow(dpy_), True, GrabModeAsync, GrabModeAsync);

    // Define which mouse clicks will send us X events.
    XGrabButton(dpy_, AnyButton, Mod4Mask, DefaultRootWindow(dpy_), True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    // Enable substructure redirection on the root window.
    XSelectInput(dpy_, DefaultRootWindow(dpy_), SubstructureNotifyMask | SubstructureRedirectMask);

    // Setup the bitch catcher.
    XSetErrorHandler(&WindowManager::OnXError);
}

void WindowManager::InitCursors() {
    cursors_[LEFT_PTR_CURSOR] = XCreateFontCursor(dpy_, XC_left_ptr);
    cursors_[RESIZE_CURSOR] = XCreateFontCursor(dpy_, XC_sizing);
    cursors_[MOVE_CURSOR] = XCreateFontCursor(dpy_, XC_fleur);
    SetCursor(DefaultRootWindow(dpy_), cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::LoadConfig() {

}


void WindowManager::Run() {
    system("displayctl && ~/.config/polybar/launch.sh");
    //system("compton --config ~/.config/compton/compton.conf &");

    for(;;) {
        // Retrieve and dispatch next X event.
        XNextEvent(dpy_, &event_);

        switch (event_.type) {
            case MapRequest:
                OnMapRequest();
                break;
            case DestroyNotify:
                OnDestroyNotify();
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
            case FocusIn:
                OnFocusIn();
                break;
            case FocusOut:
                OnFocusOut();
                break;
            default:
                break;
        }
    }
}


void WindowManager::OnMapRequest() {
    // Just map the window now. We'll discuss other things later.
    Window w = event_.xmaprequest.window;
    XMapWindow(dpy_, w);

    // Bars should not have border or be added to a workspace.
    // We check if w is a bar by inspecting its WM_CLASS.
    if (wm_utils::IsBar(dpy_, w)) {
        XWindowAttributes attr = wm_utils::QueryWindowAttributes(dpy_, w);
        bar_height_ = attr.height;
        return;
    }
    
    // Regular applications should be added to workspace client list,
    // but first we have to check if it's already in the list!
    if (!workspaces_[current_]->Has(w)) {
        // XSelectInput() and Borders are automatically done 
        // in the constructor of Client class.
        if (tiling_direction_ == Direction::HORIZONTAL) {
            workspaces_[current_]->AddHorizontal(w);
        } else {
            workspaces_[current_]->AddVertical(w);
        }
    }

    // Set the newly mapped client as the focused one.
    workspaces_[current_]->SetFocusClient(w);
    Tile(workspaces_[current_]);
}

void WindowManager::OnDestroyNotify() {
    // When a window is destroyed, remove it from the current workspace's client list.    
    Window w = event_.xdestroywindow.window;
    if (!workspaces_[current_]->Has(w)) return;
    workspaces_[current_]->Remove(w);
    Tile(workspaces_[current_]);

    // Since the previously active window has been killed, we should
    // manually set focus to another window.
    std::pair<short, short> active_client_pos = workspaces_[current_]->active_client();
    Client* c = workspaces_[current_]->GetByIndex(active_client_pos);
    if (c != nullptr) workspaces_[current_]->SetFocusClient(c->window());

    ClearNetActiveWindow();
}

void WindowManager::OnKeyPress() {
    auto modifier = event_.xkey.state;
    auto key = event_.xkey.keycode;
    std::pair<short, short> active_client_pos = workspaces_[current_]->active_client();
    Window w;
    if (active_client_pos.first >= 0) {
        w = workspaces_[current_]->GetByIndex(active_client_pos)->window();
    }

    switch (modifier) {
        case Mod1Mask:
            if (w == None) return;

            if (key >= XKeysymToKeycode(dpy_, XStringToKeysym("1"))
                    && key <= XKeysymToKeycode(dpy_, XStringToKeysym("9"))) {
                MoveWindowToWorkspace(w, key - 10);
            }
            break;

        case Mod4Mask:
            // Key pressed but does NOT require any window to be focused.
            // Mod4 + Return -> Spawn urxvt.
            if (key == XKeysymToKeycode(dpy_, XStringToKeysym("Return"))) {
                system("urxvt &");
                return;
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("d"))) {
                system("rofi -show drun");
                return;
            } else if (key >= XKeysymToKeycode(dpy_, XStringToKeysym("1"))
                    && key <= XKeysymToKeycode(dpy_, XStringToKeysym("9"))) {
                GotoWorkspace(key - 10);
                return;
            }

            if (w == None) return;

            // Mod4 + q -> Kill window.
            if (key == XKeysymToKeycode(dpy_, XStringToKeysym("q"))) {
                XKillClient(dpy_, w);
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("v"))) {
                tiling_direction_ = Direction::VERTICAL;
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("g"))) {
                tiling_direction_ = Direction::HORIZONTAL;
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("h"))) {
                workspaces_[current_]->FocusLeft();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("l"))) { 
                workspaces_[current_]->FocusRight();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("j"))) {
                workspaces_[current_]->FocusDown();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("k"))) {
                workspaces_[current_]->FocusUp();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym("f"))) {
                XRaiseWindow(dpy_, w);

                if (!fullscreen_) {
                    // Record the current window's position and size before making it fullscreen.
                    XGetWindowAttributes(dpy_, w, &attr_);
                    XMoveResizeWindow(dpy_, w, 0, 0, SCREEN_WIDTH - BORDER_WIDTH * 2, SCREEN_HEIGHT - BORDER_WIDTH * 2);
                    fullscreen_ = true;
                } else {
                    // Restore the window to its original position and size.
                    XMoveResizeWindow(dpy_, w, attr_.x, attr_.y, attr_.width, attr_.height);
                    fullscreen_ = false;
                }
            }
            break;

        default:
            break;
    }
}

void WindowManager::OnButtonPress() {
    if (event_.xbutton.subwindow == None) return;

    // Clicking on a window raises that window to the top.
    XRaiseWindow(dpy_, event_.xbutton.subwindow);
    XSetInputFocus(dpy_, event_.xbutton.subwindow, RevertToParent, CurrentTime);
    workspaces_[current_]->SetFocusClient(event_.xbutton.subwindow);

    if (event_.xbutton.state == Mod4Mask) {
        // Lookup the attributes (e.g., size and position) of a window
        // and store the result in attr_
        XGetWindowAttributes(dpy_, event_.xbutton.subwindow, &attr_);
        start_ = event_.xbutton;

        SetCursor(DefaultRootWindow(dpy_), cursors_[event_.xbutton.button]);
    }
}

void WindowManager::OnButtonRelease() {
    start_.subwindow = None;
    SetCursor(DefaultRootWindow(dpy_), cursors_[LEFT_PTR_CURSOR]);
}

void WindowManager::OnMotionNotify() {
    if (start_.subwindow == None) return;

    // Dragging a window around also raises it to the top.
    int xdiff = event_.xbutton.x - start_.x;
    int ydiff = event_.xbutton.y - start_.y;

    int new_x = attr_.x + ((start_.button == MOUSE_LEFT_BTN) ? xdiff : 0);
    int new_y = attr_.y + ((start_.button == MOUSE_LEFT_BTN) ? ydiff : 0);
    int new_width = attr_.width + ((start_.button == MOUSE_RIGHT_BTN) ? xdiff : 0);
    int new_height = attr_.height + ((start_.button == MOUSE_RIGHT_BTN) ? ydiff : 0);

    if (new_width < MIN_WINDOW_WIDTH) new_width = MIN_WINDOW_WIDTH;
    if (new_height < MIN_WINDOW_HEIGHT) new_height = MIN_WINDOW_HEIGHT;
    XMoveResizeWindow(dpy_, start_.subwindow, new_x, new_y, new_width, new_height);
}

void WindowManager::OnFocusIn() {
    SetNetActiveWindow(event_.xfocus.window);
}

void WindowManager::OnFocusOut() {
    ClearNetActiveWindow();
}


void WindowManager::SetNetActiveWindow(Window focused_window) {
    Client* c = workspaces_[current_]->Get(focused_window);
    properties_->Set(DefaultRootWindow(dpy_), properties_->net_atoms_[atom::NET_ACTIVE_WINDOW],
            XA_WINDOW, 32, PropModeReplace, (unsigned char*) &(c->window()), 1);
}

void WindowManager::ClearNetActiveWindow() {
    Atom net_active_window_atom = properties_->net_atoms_[atom::NET_ACTIVE_WINDOW];
    properties_->Delete(DefaultRootWindow(dpy_), net_active_window_atom);
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

void WindowManager::Execute(const std::string& cmd) {
    system(cmd.c_str());
}

void WindowManager::SetCursor(Window w, Cursor c) {
    XDefineCursor(dpy_, w, c);
}

void WindowManager::GotoWorkspace(short next) {
    if (current_ == next) return;
    
    workspaces_[current_]->UnmapAllClients();
    workspaces_[next]->MapAllClients();
    current_ = next;

    Tile(workspaces_[current_]);
}

void WindowManager::MoveWindowToWorkspace(Window window, short next) {    
    if (current_ == next) return;

    XUnmapWindow(dpy_, window);
    workspaces_[current_]->Remove(window);
    workspaces_[next]->AddHorizontal(window);

    if (workspaces_[current_]->ColSize() == 0) return;

    std::pair<short, short> active_client_pos = workspaces_[current_]->active_client();
    Window w = workspaces_[current_]->GetByIndex(active_client_pos)->window();

    workspaces_[current_]->SetFocusClient(w);
    workspaces_[next]->SetFocusClient(window);
    Tile(workspaces_[current_]);
}


void WindowManager::Center(Window w) {
    XWindowAttributes w_attr = wm_utils::QueryWindowAttributes(dpy_, w);
    int new_x = SCREEN_WIDTH / 2 - w_attr.width / 2;
    int new_y = SCREEN_HEIGHT / 2 - w_attr.height / 2;
    XMoveWindow(dpy_, w, new_x, new_y);
}

void WindowManager::Tile(Workspace* workspace) {
    short col_count = workspaces_[current_]->ColSize();
    if (col_count == 0) return;

    short window_width = SCREEN_WIDTH / col_count;

    for (short col = 0; col < col_count; col++) {
        short row_count = workspaces_[current_]->RowSize(col);
        short window_height = (SCREEN_HEIGHT - bar_height_) / row_count;
        
        for (short row = 0; row < row_count; row++) {
            Client* c = workspace->GetByIndex(std::pair<short, short>(col, row));
            short new_x = col * window_width + GAP_WIDTH;
            short new_y = bar_height_ + row * window_height + GAP_WIDTH;
            short new_width = window_width - BORDER_WIDTH * 2 - GAP_WIDTH;
            short new_height = window_height - BORDER_WIDTH * 2 - GAP_WIDTH;

            if (col == col_count - 1) new_width -= GAP_WIDTH;
            if (row == row_count - 1) new_height -= GAP_WIDTH;
            XMoveResizeWindow(dpy_, c->window(), new_x, new_y, new_width, new_height);
        }
    }
}
