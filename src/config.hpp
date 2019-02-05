#ifndef WMDERLAND_CONFIG_HPP_
#define WMDERLAND_CONFIG_HPP_

#include "action.hpp"
#include "util.hpp"
#include <unordered_map>
#include <vector>
#include <string>

#define WIN_MGR_NAME "Wmderland"
#define VERSION "0.9 Beta"
#define CONFIG_FILE "~/.config/Wmderland/config"
#define COOKIE_FILE "~/.local/share/Wmderland/cookie"
#define WORKSPACE_COUNT 9

#define MOUSE_LEFT_BTN 1
#define MOUSE_RIGHT_BTN 3

#define LEFT_PTR_CURSOR 0
#define MOVE_CURSOR 1
#define RESIZE_CURSOR 3

#define MIN_WINDOW_WIDTH 50
#define MIN_WINDOW_HEIGHT 50
#define DEFAULT_FLOATING_WINDOW_WIDTH 800
#define DEFAULT_FLOATING_WINDOW_HEIGHT 600

#define DEFAULT_GAP_WIDTH 15
#define DEFAULT_BORDER_WIDTH 3
#define DEFAULT_FOCUSED_COLOR 0xffffffff
#define DEFAULT_UNFOCUSED_COLOR 0xff41485f

#define VARIABLE_PREFIX "$"
#define DEFAULT_TILE_H_KEY "Mod4+g"
#define DEFAULT_TILE_V_KEY "Mod4+h"
#define DEFAULT_FOCUS_LEFT_KEY "Mod4+h"
#define DEFAULT_FOCUS_RIGHT_KEY "Mod4+l"
#define DEFAULT_FOCUS_DOWN_KEY "Mod4+j"
#define DEFAULT_FOCUS_UP_KEY "Mod4+k"
#define DEFAULT_TOGGLE_FLOATING_KEY "Mod4+o"
#define DEFAULT_TOGGLE_FULLSCREEN_KEY "Mod4+f"
#define DEFAULT_KILL_KEY "Mod4+Shift+q"
#define DEFAULT_EXIT_KEY "Mod4+Shift+Escape"

enum class ConfigKeyword {
    SET,
    ASSIGN,
    FLOATING,
    BINDSYM,
    EXEC,
    UNDEFINED
};

class Config {
public:
    static Config* GetInstance();
    virtual ~Config();

    const std::vector<Action>& GetKeybindActions(const std::string& modifier, const std::string& key) const;
    void SetKeybindActions(const std::string& modifier_and_key, const std::string& actions);

    unsigned short gap_width() const;
    unsigned short border_width() const;
    unsigned short min_window_width() const;
    unsigned short min_window_height() const;
    unsigned long focused_color() const;
    unsigned long unfocused_color() const;

    const std::unordered_map<std::string, std::string>& global_vars() const;
    const std::unordered_map<std::string, short>& spawn_rules() const;
    const std::unordered_map<std::string, bool>& float_rules() const;
    const std::unordered_map<std::string, std::vector<Action>>& keybind_rules() const;
    const std::vector<std::string>& autostart_rules() const;

private:
    static Config* instance_;
    Config(std::string filename);
    
    static ConfigKeyword StrToConfigKeyword(const std::string& s);
    const std::string& ReplaceSymbols(std::string& s);
    std::unordered_map<std::string, std::string> symtab_;

    // Global variables
    unsigned short gap_width_;
    unsigned short border_width_;
    unsigned short min_window_width_;
    unsigned short min_window_height_;
    unsigned long focused_color_;
    unsigned long unfocused_color_;

    // Rules
    std::unordered_map<std::string, std::string> global_vars_; // border width, color, etc.
    std::unordered_map<std::string, short> spawn_rules_; // spawn certain apps in certain workspaces.
    std::unordered_map<std::string, bool> float_rules_; // start certain apps in floating mode.
    std::unordered_map<std::string, std::vector<Action>> keybind_rules_; // keybind actions.
    std::vector<std::string> autostart_rules_; // launch certain apps when wm starts.
};

#endif
