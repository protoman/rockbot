#ifndef PLAYER_H
#define PLAYER_H

#include <genesis.h>

// Hitbox relative to the sprite origin.
// Source art is 29x29 (file_player_v3_1.sprite_size), padded to 32x32.
#define PLAYER_BOX_L 6
#define PLAYER_BOX_T 4
#define PLAYER_BOX_R 25
#define PLAYER_BOX_B 29

#define PLAYER_PAL PAL2

// Animation rows in res/player.png -- order must match tools/convert_player.py
#define ANIM_STAND    0
#define ANIM_WALK     1
#define ANIM_JUMP     2
#define ANIM_ATTACK   3
#define ANIM_HIT      4
#define ANIM_TELEPORT 5
#define ANIM_STAIRS   6

typedef struct {
    Sprite *spr;
    // World pixel position as plain s16. NOT fix16: SGDK fix16 is 10.6
    // format (integer range only ±512) and the stage is 3200px wide.
    s16 x, y;
    fix16 vx, vy;   // speeds (|v| <= 6, fits fix16 ±512 fine)
    bool on_ground;
    bool on_ladder;
    s16 facing;   // 1 = right, -1 = left
} Player;

extern Player player;

void player_init(void);

// pad = JOY_readJoypad(JOY_1)
void player_update(u16 pad);

// World-space hitbox, for camera focus and map-link overlap tests.
void player_get_box(s16 *left, s16 *top, s16 *right, s16 *bottom);

// Drop the player at a pixel position, grabbing a ladder if one is there.
// Used when a map link warps between maps.
void player_place(s16 worldX, s16 worldY);

#endif
