#include "wm.hpp"
#include <glog/logging.h>
extern "C" {
#include <stdlib.h>
}

WindowManager* WindowManager::instance_;

WindowManager* WindowManager::GetInstance() {
    if (!instance_) {
        Display* dpy;
        instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : nullptr;
    }
    return instance_;
}

WindowManager:: WindowManager(Display* dpy)
    : dpy_(dpy),
      root_window_(DefaultRootWindow(dpy)) {
    InitXEvents();
}

WindowManager::~WindowManager() {
    XCloseDisplay(dpy_);
}

void WindowManager::InitXEvents() {
    // Define which key combinations will send us X events.
    XGrabKey(dpy_, AnyKey, Mod4Mask, root_window_, True, GrabModeAsync, GrabModeAsync);

    // Define which mouse clicks will send us X events.
    XGrabButton(dpy_, AnyButton, Mod4Mask, root_window_, True,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    // Enable substructure redirection on the root window.
    XSelectInput(dpy_, root_window_, SubstructureNotifyMask | SubstructureRedirectMask);

    LOG(INFO) << "X events initialized.";
}

void WindowManager::Run() {
    XEvent event;

    for (;;) {
        XNextEvent(dpy_, &event);
        
        switch(event.type) {
            case MapRequest:
                OnMapRequest(event.xmaprequest);
                break;
            case DestroyNotify:
                OnDestroyNotify(event.xdestroywindow);
                break;
            case KeyPress:
                OnKeyPress(event.xkey);
                break;
            case ButtonPress:
                break;
            case ButtonRelease:
                break;
            case MotionNotify:
                break;
            default:
                break;
        }
    }
}


void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    XMapWindow(dpy_, e.window);
}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {

}

void WindowManager::OnKeyPress(const XKeyEvent& e) {
    LOG(INFO) << "Key pressed!";
    if (e.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("Return"))) {
        system("urxvt &");
        return;
    }
}

void WindowManager::OnButtonPress() {

}

void WindowManager::OnButtonRelease() {

}

void WindowManager::OnMotionNotify() {

}
