#ifndef WMDERLAND_CONFIG_HPP_
#define WMDERLAND_CONFIG_HPP_

#include "action.hpp"
#include "util.hpp"
#include <unordered_map>
#include <vector>
#include <string>

#define WIN_MGR_NAME "Wmderland"
#define VERSION "0.9.1 Beta"
#define CONFIG_FILE "~/.config/Wmderland/config"
#define COOKIE_FILE "~/.local/share/Wmderland/cookie"

#define WORKSPACE_UNSPECIFIED -1
#define WORKSPACE_COUNT 9

#define MIN_WINDOW_WIDTH 50
#define MIN_WINDOW_HEIGHT 50
#define DEFAULT_FLOATING_WINDOW_WIDTH 800
#define DEFAULT_FLOATING_WINDOW_HEIGHT 600

#define DEFAULT_GAP_WIDTH 15
#define DEFAULT_BORDER_WIDTH 3
#define DEFAULT_FOCUSED_COLOR 0xffffffff
#define DEFAULT_UNFOCUSED_COLOR 0xff41485f

#define VARIABLE_PREFIX "$"
#define DEFAULT_EXIT_KEY "Mod4+Shift+Escape"

enum class ConfigKeyword {
    SET,
    ASSIGN,
    FLOATING,
    PROHIBIT,
    BINDSYM,
    EXEC,
    UNDEFINED
};

class Config {
public:
    Config(Display* dpy, Properties* prop, std::string filename);
    virtual ~Config();
    
    int GetSpawnWorkspaceId(Window w) const;
    bool ShouldFloat(Window w) const;
    bool ShouldProhibit(Window w) const;
    const std::vector<Action>& GetKeybindActions(const std::string& modifier, const std::string& key) const;
    void SetKeybindActions(const std::string& modifier_and_key, const std::string& actions);

    unsigned short gap_width() const;
    unsigned short border_width() const;
    unsigned short min_window_width() const;
    unsigned short min_window_height() const;
    unsigned long focused_color() const;
    unsigned long unfocused_color() const;

    const std::unordered_map<std::string, short>& spawn_rules() const;
    const std::unordered_map<std::string, bool>& float_rules() const;
    const std::unordered_map<std::string, bool>& prohibit_rules() const;
    const std::unordered_map<std::string, std::vector<Action>>& keybind_rules() const;
    const std::vector<std::string>& autostart_rules() const;

private: 
    static ConfigKeyword StrToConfigKeyword(const std::string& s);
    static std::string ExtractWindowIdentifier(const std::string& s);
    std::vector<std::string> GeneratePossibleConfigKeys(Window w) const;
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
    std::unordered_map<std::string, short> spawn_rules_; // spawn certain apps in certain workspaces.
    std::unordered_map<std::string, bool> float_rules_; // start certain apps in floating mode.
    std::unordered_map<std::string, bool> prohibit_rules_; // apps that should be prohibit from starting.
    std::unordered_map<std::string, std::vector<Action>> keybind_rules_; // keybind actions.
    std::vector<std::string> autostart_rules_; // launch certain apps when wm starts.

    Display* dpy_;
    Properties* prop_;
};

#endif
