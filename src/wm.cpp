#include "wm.hpp"
#include "client.hpp"
#include "util.hpp"
#include <string>
#include <sstream>
#include <algorithm>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xatom.h>
#include <glog/logging.h>

using std::hex;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;

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
    properties_ = new Properties(dpy_);
    config_ = Config::GetInstance();

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


void WindowManager::Run() {
    // Autostart applications specified in config.
    for (auto s : config_->autostart_rules()) {
        system(s.c_str());
    }

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
            default:
                break;
        }
    }
}


void WindowManager::OnMapRequest() {
    // Just map the window now. We'll discuss other things later.
    Window w = event_.xmaprequest.window;
    
    // KDE Plasma Integration.
    if (wm_utils::QueryWmClass(dpy_, w) == "plasmashell") {
        XKillClient(dpy_, w);
        return;
    }
    
    XMapWindow(dpy_, w);

    // Bars should not have border or be added to a workspace.
    // We check if w is a bar by inspecting its WM_CLASS.
    if (wm_utils::IsBar(dpy_, w)) {
        XWindowAttributes attr = wm_utils::QueryWindowAttributes(dpy_, w);
        bar_height_ = attr.height;
        return;
    }


    // If this window is a dialog, resize it to floating window width / height and center it.
    bool should_float = wm_utils::IsDialogOrNotification(dpy_, w, properties_->net_atoms_);

    string wm_class = wm_utils::QueryWmClass(dpy_, w);
    string wm_class_and_name = wm_class + "," + wm_utils::QueryWmName(dpy_, w);

    // Apply application spawning rules (if exists).
    if (config_->spawn_rules().find(wm_class) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules()[wm_class]- 1;
        GotoWorkspace(target_workspace_id);
    } else if (config_->spawn_rules().find(wm_class_and_name) != config_->spawn_rules().end()) {
        short target_workspace_id = config_->spawn_rules()[wm_class_and_name] - 1;
        GotoWorkspace(target_workspace_id);
    }

    // Apply application floating rules (if exists).
    if (config_->float_rules().find(wm_class) != config_->float_rules().end()) {
        should_float = config_->float_rules()[wm_class];
    } else if (config_->float_rules().find(wm_class_and_name) != config_->float_rules().end()) {
        should_float = config_->float_rules()[wm_class_and_name];
    }


    if (should_float) {
        //XResizeWindow(dpy_, w, DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
        Center(w);
    }
    
    // Regular applications should be added to workspace client list,
    // but first we have to check if it's already in the list!
    if (!workspaces_[current_]->Has(w)) { 
        workspaces_[current_]->UnsetFocusClient();

        // XSelectInput() and Borders are automatically done in the constructor of Client class.
        workspaces_[current_]->Add(w, tiling_direction_, should_float);
    }

    // Set the newly mapped client as the focused one.
    workspaces_[current_]->SetFocusClient(w);
    Tile(workspaces_[current_]);
    SetNetActiveWindow(w);
}

void WindowManager::OnDestroyNotify() {
    // When a window is destroyed, remove it from the current workspace's client list.    
    Window w = event_.xdestroywindow.window;

    // If the client being destroyed is within current workspace,
    // remove it from current workspace's client list.
    if (workspaces_[current_]->Has(w)) {
        workspaces_[current_]->Remove(w);
        Tile(workspaces_[current_]);
        ClearNetActiveWindow();

        // Since the previously active window has been killed, we should
        // manually set focus to another window.
        pair<short, short> active_client_pos = workspaces_[current_]->active_client_pos();
        Client* c = workspaces_[current_]->GetByIndex(active_client_pos);
        if (c != nullptr) {
            workspaces_[current_]->SetFocusClient(c->window());
            SetNetActiveWindow(c->window());
        }
        workspaces_[current_]->RaiseAllFloatingClients();
    } else {
        Client* c = Client::mapper_[w];
        if (c) {
            c->workspace()->Remove(w);
            c->workspace()->RaiseAllFloatingClients();
        }
    }
}

void WindowManager::OnKeyPress() {
    auto modifier = event_.xkey.state;
    auto key = event_.xkey.keycode;
    pair<short, short> active_client_pos = workspaces_[current_]->active_client_pos();
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
            if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_KILL_CLIENT_KEY))) {
                Atom* supported_protocols;
                int num_supported_protocols;
                if (XGetWMProtocols(dpy_,
                            event_.xkey.subwindow,
                            &supported_protocols,
                            &num_supported_protocols) &&
                        (::std::find(supported_protocols,
                                     supported_protocols + num_supported_protocols,
                                     properties_->wm_atoms_[atom::WM_DELETE]) !=
                         supported_protocols + num_supported_protocols)) {
                    // 1. Construct message.
                    XEvent msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.xclient.type = ClientMessage;
                    msg.xclient.message_type = properties_->wm_atoms_[atom::WM_PROTOCOLS];
                    msg.xclient.window = event_.xkey.subwindow;
                    msg.xclient.format = 32;
                    msg.xclient.data.l[0] = properties_->wm_atoms_[atom::WM_DELETE];
                    // 2. Send message to window to be closed.
                    CHECK(XSendEvent(dpy_, event_.xkey.subwindow, false, 0, &msg));
                } else {
                    XKillClient(dpy_, event_.xkey.subwindow);
                }
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_TOGGLE_CLIENT_FLOAT_KEY))) {
                Client* c = workspaces_[current_]->active_client();
                if (c) {
                    c->set_floating(!c->is_floating());
                    if (c->is_floating()) {
                        XResizeWindow(dpy_, c->window(), DEFAULT_FLOATING_WINDOW_WIDTH, DEFAULT_FLOATING_WINDOW_HEIGHT);
                        Center(c->window());
                    }
                    Tile(workspaces_[current_]);
                }
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_TILE_V_KEY))) {
                tiling_direction_ = Direction::VERTICAL;
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_TILE_H_KEY))) {
                tiling_direction_ = Direction::HORIZONTAL;
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_FOCUS_LEFT_KEY))) {
                workspaces_[current_]->FocusLeft();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_FOCUS_RIGHT_KEY))) { 
                workspaces_[current_]->FocusRight();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_FOCUS_DOWN_KEY))) {
                workspaces_[current_]->FocusDown();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_FOCUS_UP_KEY))) {
                workspaces_[current_]->FocusUp();
            } else if (key == XKeysymToKeycode(dpy_, XStringToKeysym(DEFAULT_FULLSCREEN_KEY))) {
                XRaiseWindow(dpy_, w);

                if (!fullscreen_) {
                    // Record the current window's position and size before making it fullscreen.
                    XGetWindowAttributes(dpy_, w, &attr_);
                    XMoveResizeWindow(dpy_, w, 0, 0, SCREEN_WIDTH - config_->border_width() * 2, SCREEN_HEIGHT - config_->border_width() * 2);
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
    Window w = event_.xbutton.subwindow;
    if (w == None) return;

    Client* c = Client::mapper_[w];
    if (!c || !c->is_floating()) {
        return;
    } 

    if (event_.xbutton.state == Mod4Mask) {
        // Lookup the attributes (e.g., size and position) of a window
        // and store the result in attr_
        XGetWindowAttributes(dpy_, w, &attr_);
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

void WindowManager::SetCursor(Window w, Cursor c) {
    XDefineCursor(dpy_, w, c);
}

void WindowManager::GotoWorkspace(short next) {
    if (current_ == next) return;

    ClearNetActiveWindow();

    workspaces_[current_]->UnmapAllClients();
    workspaces_[next]->MapAllClients();
    current_ = next;

    Client* active_client = workspaces_[current_]->active_client();
    if (active_client) {
        workspaces_[current_]->SetFocusClient(active_client->window());
        SetNetActiveWindow(active_client->window());
        Tile(workspaces_[current_]);
    }
}

void WindowManager::MoveWindowToWorkspace(Window window, short next) {    
    if (current_ == next) return;

    XUnmapWindow(dpy_, window);
    workspaces_[current_]->Move(window, workspaces_[next]);

    if (workspaces_[current_]->ColSize() == 0) return;

    pair<short, short> active_client_pos = workspaces_[current_]->active_client_pos();
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
    // Retrieve all clients that we should tile.
    vector<vector<Client*> > tiling_clients = workspace->GetTilingClients();

    // If there's no clients to tile, return immediately.
    if (tiling_clients.empty()) {
        return;
    }

    // Tile the clients that we should tile.
    short gap_width = config_->gap_width();
    short border_width = config_->border_width();

    size_t col_count = tiling_clients.size();
    short window_width = SCREEN_WIDTH / col_count;

    for (size_t col = 0; col < col_count; col++) {
        size_t row_count = tiling_clients[col].size();
        short window_height = (SCREEN_HEIGHT - bar_height_) / row_count; 

        for (size_t row = 0; row < row_count; row++) {
            Client* c = tiling_clients[col][row];
            short new_x = col * window_width + gap_width / 2;
            short new_y = bar_height_ + row * window_height + gap_width / 2;
            short new_width = window_width - border_width * 2 - gap_width;
            short new_height = window_height - border_width * 2 - gap_width;

            if (col == 0) {
                new_x += gap_width / 2;
                new_width -= gap_width / 2;
            }

            if (row == 0) {
                new_y += gap_width / 2;
                new_height -= gap_width / 2;
            }

            if (col == col_count - 1) {
                new_width -= gap_width / 2;
            }            

            if (row == row_count - 1) {
                new_height -= gap_width / 2;
            }
            
            XMoveResizeWindow(dpy_, c->window(), new_x, new_y, new_width, new_height);
        }
    }


    // Make sure floating clients are at the top.
    vector<Client*> floating_clients = workspace->GetFloatingClients();
    if (!floating_clients.empty()) {
        workspace->UnsetFocusClient();
        workspace->SetFocusClient(floating_clients.back()->window());
    }
}
