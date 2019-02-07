#ifndef WMDERLAND_WM_HPP_
#define WMDERLAND_WM_HPP_

#include "properties.hpp"
#include "workspace.hpp"
#include "config.hpp"
#include "cookie.hpp"
#include "util.hpp"
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
}
#include <memory>
#include <vector>

class WindowManager {
public:
    static std::unique_ptr<WindowManager> GetInstance();
    virtual ~WindowManager();
    void Run();

private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    void InitWorkspaces(int count);
    void InitProperties();
    void InitXEvents();
    void InitCursors();
 
    // XEvent handlers
    void OnMapRequest(const XMapRequestEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XButtonEvent& e);
    static int OnXError(Display* dpy, XErrorEvent* e);

    bool IsDock(Window w);
    bool IsDialog(Window w);
    bool IsSplash(Window w);
    bool IsNotification(Window w);
    bool IsFullscreen(Window w);

    // Resolution and tiling area
    bool HasResolutionChanged();
    void UpdateTilingArea();
    void RestoreWindowPosSize(Window w, const Area& cookie_area, const XSizeHints& hints);

    // Docks, bars and notifications
    void MapDocksAndBars();
    void UnmapDocksAndBars();
    void RaiseAllNotificationWindows();

    // Properties manipulation
    void UpdateWindowWmState(Window w, unsigned long state);
    void SetNetActiveWindow(Window w);
    void ClearNetActiveWindow();

    // Workspace manipulation
    void GotoWorkspace(int next);
    void MoveWindowToWorkspace(Window window, int next); 

    // Client manipulation
    void Center(Window w);
    void Tile(Workspace* workspace);
    void ToggleFloating(Window w);
    void ToggleFullscreen(Window w);
    void KillClient(Window w);
    
    Display* dpy_;
    Window root_window_;
    Cursor cursors_[4];

    std::unique_ptr<Properties> prop_;
    std::unique_ptr<Config> config_;
    std::unique_ptr<Cookie> cookie_;

    std::pair<int, int> display_resolution_;
    Area tiling_area_;

    // The floating windows vector contain windows that should not be tiled but
    // must be kept on the top, e.g., bar, dock, notifications, etc.
    std::vector<Window> docks_and_bars_;
    std::vector<Window> notifications_;

    // Workspaces contain clients, where a client is a window that can be tiled
    // by the window manager.
    Workspace* workspaces_[WORKSPACE_COUNT];
    short current_;

    // Window move, resize event cache.
    XButtonEvent btn_pressed_event_;
};

#endif
