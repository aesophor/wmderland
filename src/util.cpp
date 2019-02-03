#include "util.hpp"
#include "properties.hpp"
#include <sstream>

using std::pair;
using std::size_t;
using std::string;
using std::vector;
using tiling::Action;

Area::Area() {
    Area(0, 0, 0, 0);
}

Area::Area(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {}

bool Area::operator==(const Area& other) {
    return (x == other.x) && (y == other.y) && (width == other.width) && (height == other.height);
}

bool Area::operator!=(const Area& other) {
    return (x != other.x) || (y != other.y) || (width != other.width) || (height != other.height);
}



namespace wm_utils {
 
    pair<int, int> GetDisplayResolution(Display* dpy, Window root) {
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

    XClassHint QueryWmClass(Display* dpy, Window w) {
        XClassHint hint;
        XGetClassHint(dpy, w, &hint);
        return hint;
    }

    string QueryWmName(Display* dpy, Window w) {
        Atom prop = XInternAtom(dpy, "WM_NAME", False), type;
        int form;
        unsigned long remain, len;
        unsigned char *list;

        if (XGetWindowProperty(dpy, w, prop, 0, 1024, False, AnyPropertyType,
                    &type,&form,&len,&remain,&list) != Success) {
            return NULL;
        }

        return string((char*) list);
    }

    bool QueryWindowProperty(Display* dpy, Window w, Atom src, Atom* targets, int target_count) {
        Atom prop, da;
        unsigned char *prop_ret = nullptr;
        int di;
        unsigned long dl;

        if (XGetWindowProperty(dpy, w, src, 0, sizeof(Atom), False, XA_ATOM, &da, &di, &dl, &dl, &prop_ret) == Success) {
            if (prop_ret) {
                prop = ((Atom*) prop_ret)[0];

                for (int i = 0; i < target_count; i++) {
                    if (prop == targets[i]) {
                        return true;
                    }
                }
            }
        }
        return false;
    }


    string KeysymToStr(Display* dpy, unsigned int keycode, bool shift) {
        return string(XKeysymToString(XkbKeycodeToKeysym(dpy, keycode, 0, shift))); 
    }

    unsigned int StrToKeycode(Display* dpy, const string& key_name) {
        return XKeysymToKeycode(dpy, XStringToKeysym(key_name.c_str()));
    }
    
    string KeymaskToStr(int modifier) {
        string modifier_str = "";

        switch (modifier) {
            case Mod4Mask: // Cmd
                modifier_str = "Mod4";
                break;
            case Mod4Mask | ShiftMask: // Cmd + Shift
                modifier_str = "Mod4+Shift";
                break;

            case Mod1Mask: // Alt
                modifier_str = "Mod1";
                break;
            case Mod1Mask | ShiftMask: // Alt + Shift
                modifier_str = "Mod1+Shift";
                break;

            case Mod2Mask:
                modifier_str = "Mod2";
                break;
            case Mod2Mask | ShiftMask:
                modifier_str = "Mod2+Shift";
                break;

            case Mod3Mask:
                modifier_str = "Mod3";
                break;
            case Mod3Mask | ShiftMask:
                modifier_str = "Mod3+Shift";
                break;

            case Mod5Mask:
                modifier_str = "Mod5";
                break;
            case Mod5Mask | ShiftMask:
                modifier_str = "Mod5+Shift";
                break;

            default:
                break;
        }

        return modifier_str;
    }

    int StrToKeymask(const string& modifier, bool shift) {
        static int mod_masks[6] = { 0, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask };
        int modifier_id = modifier.at(3) - '0'; // "Mod4": 0123

        if (modifier_id >= 1 && modifier_id <= 5) {
            int mod_mask = mod_masks[modifier_id];
            return (shift) ? mod_mask | ShiftMask : mod_mask;
        } else {
            return None;
        }
    }

    Action StrToAction(const string& action_str) {
        if (action_str == "tile_horizontally") {
            return Action::TILE_H;
        } else if (action_str == "tile_vertically") {
            return Action::TILE_V;
        } else if (action_str == "focus_left") {
            return Action::FOCUS_LEFT;
        } else if (action_str == "focus_right") {
            return Action::FOCUS_RIGHT;
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
        } else if (action_str == "exit") {
            return Action::EXIT;
        } else if (string_utils::StartsWith(action_str, "exec")) {
            return Action::EXEC;
        } else {
            return Action::UNDEFINED;
        }
    }



    // Rename this shit
    // We are checking if the window has _NET_WM_STATE_FULLSCREEN
    // if so, we have to set it to fullscreen upon receiving map request.
    bool IsFullScreen(Display* dpy, Window w, Atom* atoms) {
        Atom prop, da;
        unsigned char *prop_ret = nullptr;
        int di;
        unsigned long dl;

        if (XGetWindowProperty(dpy, w, atoms[atom::NET_WM_STATE], 0,
                    sizeof(Atom), False, XA_ATOM, &da, &di, &dl, &dl, &prop_ret) == Success) {
            if (prop_ret) {
                prop = ((Atom*) prop_ret)[0];
                if (prop == atoms[atom::NET_WM_STATE_FULLSCREEN]) {
                    return true;
                }
            }
        }
        return false;
    }

    bool IsDialogOrNotification(Display* dpy, Window w, Atom* atoms) {
        Atom prop, da;
        unsigned char *prop_ret = nullptr;
        int di;
        unsigned long dl;

        if (XGetWindowProperty(dpy, w, atoms[atom::NET_WM_WINDOW_TYPE], 0,
                    sizeof(Atom), False, XA_ATOM, &da, &di, &dl, &dl, &prop_ret) == Success) {
            if (prop_ret) {
                prop = ((Atom*) prop_ret)[0];
                if (prop == atoms[atom::NET_WM_WINDOW_TYPE_DIALOG] ||
                        prop == atoms[atom::NET_WM_WINDOW_TYPE_NOTIFICATION]) {
                    return true;
                }
            }
        }
        return false;
    }
 
}


namespace string_utils {

    vector<string> Split(const string& s, const char delimiter) {
        std::stringstream ss(s);
        string t;
        vector<string> tokens;

        while (std::getline(ss, t, delimiter)) {
            if (t.length() > 0) {
                tokens.push_back(t);
            }
        }
        return tokens;
    }

    vector<string> Split(const string& s, const char delimiter, int count) {
        vector<string> tokens;
        string::size_type head = 0;
        string::size_type tail = s.find(delimiter, head);

        for (short i = 0; i < count; i++) {
            string t = s.substr(head, tail - head);
            if (t.length() > 0) {
                tokens.push_back(t);
            }
            head = tail + 1;
            tail = s.find(delimiter, tail + 1);
        }

        if (head != 0) {
            string t = s.substr(head, string::npos);
            if (t.length() > 0) {
                tokens.push_back(t);
            }
        }
        return tokens;
    }

    bool StartsWith(const string& s, const string& keyword) {
        return s.find(keyword) == 0;
    }

    bool Contains(const string& s, const string& keyword) {
        return s.find(keyword) != string::npos;
    }

    void Replace(string& s, const string keyword, const string newword) {
        string::size_type pos = s.find(keyword);

        while(pos != std::string::npos) {
            s.replace(pos, keyword.size(), newword);
            pos = s.find(keyword, pos + newword.size());
        }
    }

    void Trim(string& s) {
        s.erase(s.find_last_not_of(" \n\r\t") + 1);
    }

    string ToAbsPath(const string& path) {
        string abs_path = path;

        if (path.at(0) == '~') {
            abs_path = string(getenv("HOME")) + path.substr(1, string::npos);
        }

        return abs_path;
    }

}
