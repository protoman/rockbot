#ifndef PLAYER_H
#define PLAYER_H

#include <genesis.h>

#include "player_data.h"   // PLAYER_WEAPON_N

// Hitbox relative to the sprite origin.
// Source art is 29x29 (file_player_v3_1.sprite_size), padded to 32x32.
#define PLAYER_BOX_L 6
#define PLAYER_BOX_T 4
#define PLAYER_BOX_R 25
#define PLAYER_BOX_B 29

#define PLAYER_PAL PAL2

// PC shareddata: PLAYER_INITIAL_HP, invulnerability window after a hit.
#define PLAYER_INITIAL_HP    28
#define PLAYER_INVULN_FRAMES 90

// Animation rows in res/player.png -- order must match tools/convert_player.py
#define ANIM_STAND    0
#define ANIM_WALK     1
#define ANIM_JUMP     2
#define ANIM_ATTACK   3
#define ANIM_HIT      4
#define ANIM_TELEPORT 5
#define ANIM_STAIRS   6
#define ANIM_SLIDE    7
#define ANIM_STAIRS_ATTACK 8
#define ANIM_SHIELD   9

typedef struct {
    Sprite *spr;
    // World pixel position as plain s16. NOT fix16: SGDK fix16 is 10.6
    // format (integer range only ±512) and the stage is 3200px wide.
    s16 x, y;
    fix16 vx, vy;   // speeds (|v| <= 6, fits fix16 ±512 fine)
    bool on_ground;
    bool on_ladder;
    s16 facing;   // 1 = right, -1 = left
    u8 hp;        // 0 = death (respawns with full hp)
    u16 invuln;   // frames of damage immunity + blink
    // slide (PC: dash button on the ground, character.cpp:1672; needs a
    // fresh press — _dash_button_released)
    bool sliding;
    bool slide_prev;
    s16 slide_dist;
    // shield (PC ANIM_TYPE_SHIELD, ROCKBOT only): standing still + the shield
    // gesture held; blocks enemy shots and forbids attacking
    bool shielding;
    // weapon (PC selected_weapon): drives the shot (weapon_data[]) and the
    // palette swap on indices 13/14/15 (player_data[].wcol[])
    u8 weapon;
    u8 ammo[PLAYER_WEAPON_N];   // PC game_save.items.weapons[] gauge (0..28)
    // charge shot (PC check_must_attack: press = normal, hold >= 1000ms
    // = semi, >= 2200ms = full, fire on release)
    u16 charge_frames;
    bool fire_prev;
    // hit pushback (PC ANIM_TYPE_HIT, character.cpp:194): remaining px
    // away from the facing direction + sub-pixel carry + push direction
    fix16 hit_push;
    fix16 hit_acc;
    s16 hit_dir;
} Player;

extern Player player;

void player_init(void);

// Selected player 0..3 (player-select screen). Changes the sprite sheet +
// palette used in-game (each player has different weapon colours/abilities).
void player_set_number(u8 n);
const SpriteDefinition *player_definition(void);

// Per-player abilities (from player_list_v3_1_1.dat, src/player_data.c):
// see the M16 notes in stage0_plan.md for the per-player breakdown.
u8 player_max_shots(void);
u8 player_simultaneous(void);
s8 player_damage_mod(void);
bool player_can_charge(void);
bool player_can_double_jump(void);
bool player_can_slide(void);
bool player_can_diagonal(void);
bool player_can_air_dash(void);
bool player_can_shield(void);

// TRUE while the shield is up (ROCKBOT); enemy shots are blocked (eshot.c).
bool player_is_shielding(void);

// Weapon (PC selected_weapon / weapon_colors[selected_weapon]).
u8 player_weapon(void);
void player_set_weapon(u8 n);
u8 player_weapon_ammo(void);        // 0 = can't fire (PC classplayer.cpp:336)
void player_use_weapon_ammo(void);  // one unit per shot
void player_refill_weapons(void);   // PC refill_weapons(), at stage start

// Lives (PC game_save.items.lifes): one per death, 5 at stage start. When they
// run out the caller should leave the stage (PC: game over).
u8 player_lives(void);
bool player_out_of_lives(void);

// pad = JOY_readJoypad(JOY_1)
void player_update(u16 pad);

// World-space hitbox, for camera focus and map-link overlap tests.
void player_get_box(s16 *left, s16 *top, s16 *right, s16 *bottom);

// Drop the player at a pixel position, grabbing a ladder if one is there.
// Used when a map link warps between maps.
void player_place(s16 worldX, s16 worldY);

// Re-place the player sprite for the current camera (after player_place +
// stage_arrival_camera, so it does not flash at its old screen position).
void player_sync_sprite(void);

// Reload the player palette (PAL2). Needed after a full-palette fade, which
// blacks out PAL2 as well and nothing else restores it.
void player_restore_palette(void);

// The player's current palette definition (PAL2 source), for building a
// full-CRAM fade buffer.
const Palette *player_palette_def(void);

// Contact/projectile damage. Respects the invulnerability window; at hp 0
// respawns at the stage start with full hp.
void player_take_damage(u8 amount);

// Instant death, ignoring invulnerability: a platform pressing the player
// into a solid tile (PC damage(999)).
void player_crush(void);

// Record the respawn point (worldX, feet-Y worldY) when touching a
// checkpoint object. Death and pit-falls respawn there while on that map.
void player_set_checkpoint(s16 worldX, s16 worldY);

// Hide/show the player sprite (in-game menu overlay).
void player_set_visible(bool visible);

#endif
