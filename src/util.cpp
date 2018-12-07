#include "util.hpp"
#include "properties.hpp"
#include <sstream>

using std::pair;
using std::size_t;
using std::string;
using std::vector;

namespace wm_utils {

    pair<short, short> GetDisplayResolution(Display* dpy, Window root) {
        XWindowAttributes root_attr = QueryWindowAttributes(dpy, root);
        return pair<short, short>(root_attr.width, root_attr.height);
    }

    XWindowAttributes QueryWindowAttributes(Display* dpy, Window w) {
        XWindowAttributes ret;
        XGetWindowAttributes(dpy, w, &ret);
        return ret;
    }

    XSizeHints QueryWmNormalHints(Display* dpy, Window w) {
        XSizeHints hints;
        long msize;
        XGetWMNormalHints(dpy, w, &hints, &msize);
        return hints;
    }

    string QueryWmClass(Display* dpy, Window w) {
        XClassHint hint;
        XGetClassHint(dpy, w, &hint);
        return string(hint.res_class);
    }

    string QueryWmName(Display* dpy, Window w) {
        XClassHint hint;
        XGetClassHint(dpy, w, &hint);
        return string(hint.res_name);
    }

    unsigned int QueryKeycode(Display* dpy, const string& key_name) {
        return XKeysymToKeycode(dpy, XStringToKeysym(key_name.c_str()));
    }

    string QueryKeysym(Display* dpy, unsigned int keycode, bool shift) {
        return string(XKeysymToString(XkbKeycodeToKeysym(dpy, keycode, 0, shift))); 
    }

    Action StrToAction(const string& action_str) {
        if (action_str == "tile_horizontally") {
            return TILE_H;
        } else if (action_str == "tile_vertically") {
            return TILE_V;
        } else if (action_str == "focus_left") {
            return FOCUS_LEFT;
        } else if (action_str == "focus_right") {
            return FOCUS_RIGHT;
        } else if (action_str == "focus_down") {
            return Action::FOCUS_DOWN;
        } else if (action_str == "focus_up") {
            return Action::FOCUS_UP;
        } else if (action_str == "toggle_floating") {
            return Action::TOGGLE_FLOATING;
        } else if (action_str == "toggle_fullscreen") {
            return Action::TOGGLE_FULLSCREEN;
        } else if (action_str == "kill") {
            return Action::KILL;
        } else if (string_utils::StartsWith(action_str, "exec")) {
            return Action::EXEC;
        } else {
            return Action::UNDEFINED;
        }
    }


    bool IsDialogOrNotification(Display* dpy, Window w, Atom* atoms) {
        Atom prop, da;
        unsigned char *prop_ret = nullptr;
        int di;
        unsigned long dl;

        if (XGetWindowProperty(dpy, w, atoms[atom::NET_WM_WINDOW_TYPE], 0,
                    sizeof (Atom), False, XA_ATOM, &da, &di, &dl, &dl, &prop_ret) == Success) {
            if (prop_ret) {
                prop = ((Atom *)prop_ret)[0];
                if (prop == atoms[atom::NET_WM_WINDOW_TYPE_DIALOG] ||
                        prop == atoms[atom::NET_WM_WINDOW_TYPE_NOTIFICATION]) {
                    return true;
                }
            }
        }

        return false;
    }

    bool IsBar(const string& wm_class) {
        return wm_class.find("Polybar") != string::npos;
    }

    bool IsBar(Display* dpy, Window w) {
        return IsBar(QueryWmClass(dpy, w));
    }

}


namespace string_utils {

    vector<string> Split(const string& s, const char delimiter) {
        std::stringstream ss(s);
        string t;
        vector<string> tokens;

        while (std::getline(ss, t, delimiter)) {
            tokens.push_back(t);
        }
        return tokens;
    }

    vector<string> Split(const string& s, const char delimiter, short count) {
        vector<string> tokens;
        string::size_type head = 0;
        string::size_type tail = s.find(delimiter, head);

        for (short i = 0; i < count; i++) {
            tokens.push_back(s.substr(head, tail - head));
            head = tail + 1;
            tail = s.find(delimiter, tail + 1);
        }

        if (head != 0) {
            tokens.push_back(s.substr(head, string::npos));
        }
        return tokens;
    }

    bool StartsWith(const string& s, const string& keyword) {
        return s.find(keyword) == 0;
    }

    bool Contains(const string& s, const string& keyword) {
        return s.find(keyword) != string::npos;
    }

    void Trim(string& s) {
        s.erase(s.find_last_not_of(" \n\r\t") + 1);
    }

}
