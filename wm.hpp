#ifndef WM_HPP_
#define WM_HPP_

#include "workspace.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>
#include <vector>
#include <memory>

#define WM_NAME "Wmderland"
#define VERSION 0.1

#define MOUSE_LEFT_BTN 1
#define MOUSE_RIGHT_BTN 3

#define MIN_WINDOW_WIDTH 50
#define MIN_WINDOW_HEIGHT 50
#define SCREEN_WIDTH 1680
#define SCREEN_HEIGHT 1050

#define BORDER_WIDTH 3
#define FOCUSED_COLOR 0xffffff
#define UNFOCUSED_COLOR 0x41485f

#define WORKSPACE_COUNT 8

class WindowManager {
public:
    static std::unique_ptr<WindowManager> GetInstance();
    ~WindowManager();
    void Run();
private:
    WindowManager(Display* dpy);
    void OnCreateNotify();
    void OnDestroyNotify();
    void OnMapRequest();
    void OnKeyPress();
    void OnButtonPress();
    void OnButtonRelease();
    void OnMotionNotify();
    void OnFocusIn();
    void OnFocusOut();

    void GotoWorkspace(int n);
    
    Display* dpy_;
    XEvent event_;
    XWindowAttributes attr_;
    XButtonEvent start_;
    bool fullscreen_;

    std::vector<Workspace*> workspaces_;
    short current_workspace_;
};

#endif
