#pragma once

// Shared by the fixed-LCD ports (Vita, Switch): where the game frame goes on the screen.
// Integer scale only (a fractional factor draws some pixel rows taller than others, which distorts fonts
// and the HUD bars), centered with black borders. Returns false when the frame is larger than the screen.

#include <SDL.h>

namespace present_rect {

inline bool integer_scale(int lcd_w, int lcd_h, int game_w, int game_h, SDL_Rect *dst)
{
    if (game_w <= 0 || game_h <= 0) {
        return false;
    }
    int scale = lcd_w / game_w;
    if (lcd_h / game_h < scale) {
        scale = lcd_h / game_h;
    }
    if (scale < 1) {
        return false;
    }
    dst->w = game_w * scale;
    dst->h = game_h * scale;
    dst->x = (lcd_w - dst->w) / 2;
    dst->y = (lcd_h - dst->h) / 2;
    return true;
}

} // namespace present_rect
