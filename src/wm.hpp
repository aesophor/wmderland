#ifndef WM_HPP_
#define WM_HPP_

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
    // Singleton since only a single WM is required at once.
    static WindowManager* GetInstance();
    ~WindowManager();
    void Run();
    void Stop();

private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    void InitWorkspaces(short count);
    void InitProperties();
    void InitXEvents();
    void InitCursors();
    void SetCursor(Window w, Cursor c);

    // XEvent handlers
    static int OnXError(Display* dpy, XErrorEvent* e);
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
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
    Window root_;

    // Window move, resize event cache.
    XButtonEvent btn_pressed_event_;

    Cursor cursor_;

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
