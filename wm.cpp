#include "wm.hpp"
#include "util.hpp"

std::unique_ptr<WindowManager> WindowManager::GetInstance() {
    Display* dpy;    
    return ((dpy = XOpenDisplay(None)) == None) ? 
        None : std::unique_ptr<WindowManager>(new WindowManager(dpy));
}

WindowManager::WindowManager(Display* dpy) {
    dpy_ = dpy;
    fullscreen_ = false;

    // Set _NET_WM_NAME.
    XChangeProperty(dpy_, DefaultRootWindow(dpy_),
            XInternAtom(dpy_, "_NET_WM_NAME", False),
            XInternAtom(dpy_, "UTF8_STRING", False),
            8, PropModeReplace, (unsigned char *) WM_NAME, sizeof(WM_NAME));

    // Define which key combinations will send us X events.
    XGrabKey(dpy_, AnyKey, Mod4Mask, DefaultRootWindow(dpy_), True, GrabModeAsync, GrabModeAsync);

    // Define which mouse clicks will send us X events.
    XGrabButton(dpy_, AnyButton, AnyModifier, DefaultRootWindow(dpy_), True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    XSelectInput(dpy_, DefaultRootWindow(dpy_), SubstructureNotifyMask | SubstructureRedirectMask);
}

WindowManager::~WindowManager() {
    XCloseDisplay(dpy_);
}


void WindowManager::Run() {
    system("displayctl && ~/.config/polybar/launch.sh");
    
    for(;;) {
        // Retrieve and dispatch next X event.
        XNextEvent(dpy_, &event_);
        
        switch (event_.type) {
            case MapRequest:
                OnMapRequest();
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
    Window w = event_.xmaprequest.window;
    XClassHint hint;
    XGetClassHint(dpy_, w, &hint);

    // Don't apply border on polybar.
    if (strcmp(hint.res_class, "Polybar") != 0) {
        XSetWindowBorderWidth(dpy_, w, BORDER_WIDTH);
        XSetWindowBorder(dpy_, w, FOCUSED_COLOR);
    }

    XMapWindow(dpy_, w);
    XRaiseWindow(dpy_, w);
    XSetInputFocus(dpy_, w, RevertToParent, CurrentTime);
}

void WindowManager::OnKeyPress() {
    // Key pressed but does NOT require any window to be focused.
    // Mod4 + Return -> Spawn urxvt.
    if (event_.xkey.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("Return"))) {
        system("urxvt &");
        return;
    } else if (event_.xkey.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("d"))) {
        system("rofi -show drun");
        return;
    }

    if (event_.xkey.subwindow == None) {
        return;
    }


    // Mod4 + q -> Kill window.
    if (event_.xkey.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("q"))) {
        XKillClient(dpy_, event_.xkey.subwindow);
    } else if (event_.xkey.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("f"))) {
        XRaiseWindow(dpy_, event_.xkey.subwindow);

        if (!fullscreen_) {
            // Record the current window's position and size before making it fullscreen.
            XGetWindowAttributes(dpy_, event_.xkey.subwindow, &attr_);
            XMoveResizeWindow(dpy_, event_.xkey.subwindow, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            fullscreen_ = true;
        } else {
            // Restore the window to its original position and size.
            XMoveResizeWindow(dpy_, event_.xkey.subwindow, attr_.x, attr_.y, attr_.width, attr_.height);
            fullscreen_ = false;
        }
    }
}

void WindowManager::OnButtonPress() {
    if (event_.xbutton.subwindow == None) {
        return;
    }

    // Clicking on a window raises that window to the top.
    XRaiseWindow(dpy_, event_.xbutton.subwindow);
    XSetInputFocus(dpy_, event_.xbutton.subwindow, RevertToParent, CurrentTime);

    if (event_.xbutton.state == Mod4Mask) {
        // Lookup the attributes (e.g., size and position) of a window
        // and store the result in attr_
        XGetWindowAttributes(dpy_, event_.xbutton.subwindow, &attr_);
        start_ = event_.xbutton;
    }
}

void WindowManager::OnButtonRelease() {
    start_.subwindow = None;
}

void WindowManager::OnMotionNotify() {
    if (start_.subwindow == None) {
        return;
    }

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
