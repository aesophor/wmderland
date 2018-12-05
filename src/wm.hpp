#ifndef WM_HPP_
#define WM_HPP_

#include "properties.hpp"
#include "workspace.hpp"
#include "config.hpp"
#include "util.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cstring>
#include <vector>

class WindowManager {
public:
    /* Singleton since only a single WM is required at once. */
    static WindowManager* GetInstance();
    ~WindowManager();
    void Run();
private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);

    /* Setup */
    void InitWorkspaces(short count);
    void InitProperties();
    void InitXEvents();
    void InitCursors();
    void SetCursor(Window w, Cursor c);

    /* XEvent handlers */
    static int OnXError(Display* dpy, XErrorEvent* e);
    void OnMapRequest();
    void OnDestroyNotify();
    void OnKeyPress();
    void OnButtonPress();
    void OnButtonRelease();
    void OnMotionNotify();

    /* Properties manipulation */
    void SetNetActiveWindow(Window focused_window);
    void ClearNetActiveWindow();

    /* Workspace manipulation */
    void GotoWorkspace(short next);
    void MoveWindowToWorkspace(Window window, short next); 

    /* Client window placement */
    void Center(Window w);
    void Tile(Workspace* workspace);

    void KillClient(Window w);
    

    Display* dpy_;
    Window root_;
    XEvent event_;
    XWindowAttributes attr_;
    XButtonEvent start_;
    Cursor cursor_;
    bool fullscreen_;
    Direction tiling_direction_;

    /* Properties */
    Properties* properties_;
    Config* config_;

    /* Cursors */
    Cursor cursors_[4];

    /* Bar */
    short bar_height_;

    /* Workspaces */
    std::vector<Workspace*> workspaces_;
    short current_;
};

#endif
