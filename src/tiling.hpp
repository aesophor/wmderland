// In Wmderland, each actual window is represented by a leaf
// in the window tree, while an internal node contains the 
// information about how its children should be tiled (either
// unspecified, horizontally or vertically).
//
// The direction of a leaf is UNSPECIFIED by default. However,
// if the user changed it to HORIZONTAL or VERTICAL, then this
// leaf is ready to be splitted in the specified direction.

#ifndef WMDERLAND_TILING_HPP_
#define WMDERLAND_TILING_HPP_

namespace tiling {

    enum Direction {
        UNSPECIFIED,
        HORIZONTAL,
        VERTICAL
    };

    enum Action {
        TILE_H,
        TILE_V,
        FOCUS_LEFT,
        FOCUS_RIGHT,
        FOCUS_DOWN,
        FOCUS_UP,
        TOGGLE_FLOATING,
        TOGGLE_FULLSCREEN,
        KILL,
        EXIT,
        EXEC,
        UNDEFINED
    };

}

#endif
