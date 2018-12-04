#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <unordered_map>
#include <vector>
#include <string>


#define WM_NAME "Wmderland"
#define VERSION "0.5 Beta"
#define CONFIG_FILE "~/.config/Wmderland/config"
#define WORKSPACE_COUNT 9

#define MOUSE_LEFT_BTN 1
#define MOUSE_RIGHT_BTN 3

#define LEFT_PTR_CURSOR 0
#define MOVE_CURSOR 1
#define RESIZE_CURSOR 3

#define SCREEN_WIDTH 1680
#define SCREEN_HEIGHT 1050

#define MIN_WINDOW_WIDTH 50
#define MIN_WINDOW_HEIGHT 50
#define DEFAULT_GAP_WIDTH 15
#define DEFAULT_BORDER_WIDTH 3
#define DEFAULT_FOCUSED_COLOR 0xffffffff
#define DEFAULT_UNFOCUSED_COLOR 0xff41485f

#define DEFAULT_TILE_H_KEY "g"
#define DEFAULT_TILE_V_KEY "v"
#define DEFAULT_FOCUS_LEFT_KEY "h"
#define DEFAULT_FOCUS_RIGHT_KEY "l"
#define DEFAULT_FOCUS_DOWN_KEY "j"
#define DEFAULT_FOCUS_UP_KEY "k"
#define DEFAULT_FULLSCREEN_KEY "f"
#define DEFAULT_KILL_CLIENT_KEY "q"
#define DEFAULT_TOGGLE_CLIENT_FLOAT_KEY "o"


class Config {
public:
    static Config* GetInstance();

    std::string Get(const std::string& key) const;
    bool Has(const std::string& key) const;

    unsigned short gap_width();
    unsigned short border_width();
    unsigned short min_window_width();
    unsigned short min_window_height();
    unsigned long focused_color();
    unsigned long unfocused_color();

    std::unordered_map<std::string, std::string>& global_vars();
    std::unordered_map<std::string, short>& spawn_rules();
    std::vector<std::string>& autostart_rules();

private:
    static Config* instance_;
    Config(std::string filename);

    unsigned short gap_width_;
    unsigned short border_width_;
    unsigned short min_window_width_;
    unsigned short min_window_height_;
    unsigned long focused_color_;
    unsigned long unfocused_color_;

    std::unordered_map<std::string, std::string> global_vars_;
    std::unordered_map<std::string, short> spawn_rules_;
    std::vector<std::string> autostart_rules_;
};

#endif
