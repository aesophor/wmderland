#ifndef WMDERLAND_WM_HPP_
#define WMDERLAND_WM_HPP_

#include "properties.hpp"
#include "workspace.hpp"
#include "config.hpp"
#include "cookie.hpp"
#include "util.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cstring>
#include <vector>

class WindowManager {
public:
    static WindowManager* GetInstance();
    virtual ~WindowManager();
    void Run();

private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    void InitWorkspaces(short count);
    void InitProperties();
    void InitXEvents();
    void InitCursors();

    // XEvent handlers
    static int OnXError(Display* dpy, XErrorEvent* e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XButtonEvent& e);

    // Properties manipulation
    void SetNetActiveWindow(Window focused_window);
    void ClearNetActiveWindow();

    // Workspace manipulation
    void GotoWorkspace(short next);
    void MoveWindowToWorkspace(Window window, short next); 

    // Client manipulation
    void Center(Window w);
    void Tile(Workspace* workspace);
    void ToggleFloating(Window w);
    void ToggleFullScreen(Window w);
    void KillClient(Window w);
 
    Display* dpy_;
    Window root_window_;

    // Window move, resize event cache.
    XButtonEvent btn_pressed_event_;

    // Properties
    Properties* prop_;
    Config* config_;
    Cookie* cookie_;

    // Cursors
    Cursor cursors_[4];

    // Bar
    short bar_height_;
    Window bar_;

    // Workspaces
    Workspace* workspaces_[WORKSPACE_COUNT];
    short current_;
};

#endif
