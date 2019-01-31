#ifndef WMDERLAND_HPP_
#define WMDERLAND_HPP_

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

class WindowManager {
public:
    static WindowManager* GetInstance();
    virtual ~WindowManager();
    void Run();

private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    void InitXEvents();

    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XButtonEvent& e);

    Display* dpy_;
    Window root_window_;

    // Window move, resize data cache.
    XButtonEvent btn_pressed_event_;
    XWindowAttributes btn_pressed_attr_;
};

#endif
