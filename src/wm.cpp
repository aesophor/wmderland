#include "wm.hpp"
#include <glog/logging.h>
extern "C" {
#include <stdlib.h>
}

#define MOUSE_LEFT_BTN 1
#define MOUSE_RIGHT_BTN 3

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
            case CreateNotify:
                OnCreateNotify(event.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(event.xdestroywindow);
                break;
            case MapRequest:
                OnMapRequest(event.xmaprequest);
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


void WindowManager::OnCreateNotify(const XCreateWindowEvent& e) {

}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e) {

}

void WindowManager::OnMapRequest(const XMapRequestEvent& e) {
    XMapWindow(dpy_, e.window);
}

void WindowManager::OnKeyPress(const XKeyEvent& e) {
    if (e.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("d"))) {
        system("rofi -show drun &");
    } else if (e.keycode == XKeysymToKeycode(dpy_, XStringToKeysym("Return"))) {
        system("urxvt &");
        return;
    }
}

void WindowManager::OnButtonPress(const XButtonEvent& e) {
    if (!e.subwindow) return;

    if (e.state == Mod4Mask) {
        XGetWindowAttributes(dpy_, e.subwindow, &btn_pressed_attr_);
        btn_pressed_event_ = e;
    }
}

void WindowManager::OnButtonRelease(const XButtonEvent& e) {
    if (!btn_pressed_event_.subwindow) return;

    btn_pressed_event_.subwindow = None;
}

void WindowManager::OnMotionNotify(const XButtonEvent& e) {
    if (!btn_pressed_event_.subwindow) return;

    int xdiff = e.x - btn_pressed_event_.x;
    int ydiff = e.y - btn_pressed_event_.y;

    int x = btn_pressed_attr_.x + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? xdiff : 0);
    int y = btn_pressed_attr_.x + ((btn_pressed_event_.button == MOUSE_LEFT_BTN) ? ydiff : 0);
    int width = btn_pressed_attr_.width + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? xdiff : 0);
    int height = btn_pressed_attr_.height + ((btn_pressed_event_.button == MOUSE_RIGHT_BTN) ? ydiff : 0);

    if (width <= 50) width = 50;
    if (height <= 50) height = 50;

    XMoveResizeWindow(dpy_, btn_pressed_event_.subwindow, x, y, width, height);
}
