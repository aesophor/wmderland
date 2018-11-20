#include "wm.hpp"
#include "global.hpp"
#include <string>
#include <algorithm>
#include <glog/logging.h>

WindowManager* WindowManager::instance_;

WindowManager* WindowManager::GetInstance() {
    // If the instance is not yet initialized, we'll try to open a display
    // to X server. If it fails (i.e., dpy is None), then we return None
    // to the caller. Otherwise we return an instance of WindowManager.
    if (!instance_) {
        Display* dpy;
        instance_ = (dpy = XOpenDisplay(None)) ? new WindowManager(dpy) : None;
    }
    return instance_;
}

WindowManager::WindowManager(Display* dpy) {
    dpy_ = dpy;
    current_ = 0;
    fullscreen_ = false;

    // Initialize 10 workspaces.
    for (int i = 0; i < WORKSPACE_COUNT - 1; i++) {
        workspaces_.push_back(new Workspace(dpy_, i));
    }
    
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

    // Enable substructure redirection on the root window.
    XSelectInput(dpy_, DefaultRootWindow(dpy_), SubstructureNotifyMask | SubstructureRedirectMask);

    //XSetErrorHandler(&WindowManager::OnXError);
}

WindowManager::~WindowManager() {
    for (auto w : workspaces_) {
        delete w;
    }

    XCloseDisplay(dpy_);
}


void WindowManager::Run() {
    system("displayctl && ~/.config/polybar/launch.sh");

    for(;;) {
        // Retrieve and dispatch next X event.
        XNextEvent(dpy_, &event_);

        switch (event_.type) {
            case CreateNotify:
                OnCreateNotify();
                break;
            case DestroyNotify:
                OnDestroyNotify();
                break;
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
            case FocusIn:
                OnFocusIn();
                break;
            case FocusOut:
                OnFocusOut();
                break;
            default:
                break;
        }
    }
}

void WindowManager::OnCreateNotify() {

}

void WindowManager::OnDestroyNotify() {
    // When a window is destroyed, remove it from the list of windows
    // in the current workspace.
    workspaces_[current_]->Remove(event_.xdestroywindow.window);
}

void WindowManager::OnMapRequest() {
    Window w = event_.xmaprequest.window;
    XClassHint hint;
    XGetClassHint(dpy_, w, &hint);

    // Don't apply border on polybar.
    if (strcmp(hint.res_class, "Polybar") != 0) {
        XSetWindowBorderWidth(dpy_, w, BORDER_WIDTH);
        XSetWindowBorder(dpy_, w, FOCUSED_COLOR);
        LOG(INFO) << "Adding " << hint.res_class << " (" << w << ")";
        
        // If this window is already in the vector, don't re-add it.
        if (!workspaces_[current_]->Has(w)) {
            workspaces_[current_]->Add(w);
            XSelectInput(dpy_, w, FocusChangeMask);
        }
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
    } else if (event_.xkey.keycode >= XKeysymToKeycode(dpy_, XStringToKeysym("1"))
            && event_.xkey.keycode <= XKeysymToKeycode(dpy_, XStringToKeysym("9"))) {
        GotoWorkspace(event_.xkey.keycode - 10);
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
    
    //for (auto const w : workspaces_[current_workspace_]->windows) {
    //    XSetWindowBorder(dpy_, w, (w == event_.xbutton.subwindow) ? FOCUSED_COLOR : UNFOCUSED_COLOR);
    //}

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

void WindowManager::OnFocusIn() {
    XSetWindowBorder(dpy_, event_.xfocus.window, FOCUSED_COLOR);
}

void WindowManager::OnFocusOut() {
    XSetWindowBorder(dpy_, event_.xfocus.window, UNFOCUSED_COLOR);
}

std::string XRequestCodeToString(unsigned char request_code);

int WindowManager::OnXError(Display* dpy, XErrorEvent* e) {
    const int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(dpy, e->error_code, error_text, sizeof(error_text));
    LOG(ERROR) << "Received X error:\n"
        << "    Request: " << int(e->request_code)
        << " - " << XRequestCodeToString(e->request_code) << "\n"
        << "    Error code: " << int(e->error_code)
        << " - " << error_text << "\n"
        << "    Resource ID: " << e->resourceid;
    // The return value is ignored.
    return 0;
}


void WindowManager::GotoWorkspace(short next) {
    if (current_ == next) {
        return;
    }

    workspaces_[current_]->UnmapAllWindows();
    workspaces_[next]->MapAllWindows();
    current_ = next;
}

std::string XRequestCodeToString(unsigned char request_code) {
  static const char* const X_REQUEST_CODE_NAMES[] = {
      "",
      "CreateWindow",
      "ChangeWindowAttributes",
      "GetWindowAttributes",
      "DestroyWindow",
      "DestroySubwindows",
      "ChangeSaveSet",
      "ReparentWindow",
      "MapWindow",
      "MapSubwindows",
      "UnmapWindow",
      "UnmapSubwindows",
      "ConfigureWindow",
      "CirculateWindow",
      "GetGeometry",
      "QueryTree",
      "InternAtom",
      "GetAtomName",
      "ChangeProperty",
      "DeleteProperty",
      "GetProperty",
      "ListProperties",
      "SetSelectionOwner",
      "GetSelectionOwner",
      "ConvertSelection",
      "SendEvent",
      "GrabPointer",
      "UngrabPointer",
      "GrabButton",
      "UngrabButton",
      "ChangeActivePointerGrab",
      "GrabKeyboard",
      "UngrabKeyboard",
      "GrabKey",
      "UngrabKey",
      "AllowEvents",
      "GrabServer",
      "UngrabServer",
      "QueryPointer",
      "GetMotionEvents",
      "TranslateCoords",
      "WarpPointer",
      "SetInputFocus",
      "GetInputFocus",
      "QueryKeymap",
      "OpenFont",
      "CloseFont",
      "QueryFont",
      "QueryTextExtents",
      "ListFonts",
      "ListFontsWithInfo",
      "SetFontPath",
      "GetFontPath",
      "CreatePixmap",
      "FreePixmap",
      "CreateGC",
      "ChangeGC",
      "CopyGC",
      "SetDashes",
      "SetClipRectangles",
      "FreeGC",
      "ClearArea",
      "CopyArea",
      "CopyPlane",
      "PolyPoint",
      "PolyLine",
      "PolySegment",
      "PolyRectangle",
      "PolyArc",
      "FillPoly",
      "PolyFillRectangle",
      "PolyFillArc",
      "PutImage",
      "GetImage",
      "PolyText8",
      "PolyText16",
      "ImageText8",
      "ImageText16",
      "CreateColormap",
      "FreeColormap",
      "CopyColormapAndFree",
      "InstallColormap",
      "UninstallColormap",
      "ListInstalledColormaps",
      "AllocColor",
      "AllocNamedColor",
      "AllocColorCells",
      "AllocColorPlanes",
      "FreeColors",
      "StoreColors",
      "StoreNamedColor",
      "QueryColors",
      "LookupColor",
      "CreateCursor",
      "CreateGlyphCursor",
      "FreeCursor",
      "RecolorCursor",
      "QueryBestSize",
      "QueryExtension",
      "ListExtensions",
      "ChangeKeyboardMapping",
      "GetKeyboardMapping",
      "ChangeKeyboardControl",
      "GetKeyboardControl",
      "Bell",
      "ChangePointerControl",
      "GetPointerControl",
      "SetScreenSaver",
      "GetScreenSaver",
      "ChangeHosts",
      "ListHosts",
      "SetAccessControl",
      "SetCloseDownMode",
      "KillClient",
      "RotateProperties",
      "ForceScreenSaver",
      "SetPointerMapping",
      "GetPointerMapping",
      "SetModifierMapping",
      "GetModifierMapping",
      "NoOperation",
  };
  return X_REQUEST_CODE_NAMES[request_code];
}
