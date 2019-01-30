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

    void OnMapRequest(const XMapRequestEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnButtonPress();
    void OnButtonRelease();
    void OnMotionNotify();

    Display* dpy_;
    Window root_window_;
};

#endif
