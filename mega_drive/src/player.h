#ifndef PLAYER_H
#define PLAYER_H

#include <genesis.h>

#include "player_data.h"   // PLAYER_WEAPON_N

// Hitbox relative to the sprite origin.
// Source art is 29x29 (file_player_v3_1.sprite_size), padded to 32x32.
// PC player hitbox: file_player_v3_1_1.sprite_hit_area = (5,3,16,26) on the
// 29x29 art, i.e. x 5..21 and y 3..29. The old 6..25 box was 19px wide and
// did not fit a 1-tile ladder shaft: centring on the ladder left the box
// overlapping both side walls, so unstick_x pinned the player ("stuck at
// some position") and walking across a ladder top pushed him into the
// column. Ladder centring now lands the 16px box exactly on the tile.
#define PLAYER_BOX_L 5
#define PLAYER_BOX_T 3
#define PLAYER_BOX_R 21
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
#define ANIM_SPECIAL  10   // SHORYUKEN (PC ANIM_TYPE_SPECIAL_ATTACK, frame 29)

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
    // PC game_save.items.energy_tanks / weapon_tanks (stored consumables,
    // used from the in-game menu), persisted per save slot.
    u8 tanks_hp;
    u8 tanks_wp;
    u8 tanks_sp;
    u8 armor;       // bit0 ARMS, bit1 BODY, bit2 LEGS (PC game_save.armor_pieces)
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

// Stage setup: position, hp, weapons, palette, sprite. Persistent progress
// (lives/tanks/armor) is NOT touched — see player_new_game/player_set_progress.
void player_init(void);

// NEW GAME: PC reset_lifes() defaults (5 on normal) and empty inventory.
void player_new_game(void);
// Restore progress from a save slot (values clamped to the PC limits).
void player_set_progress(u8 lives, u8 tanks_hp, u8 tanks_wp, u8 tanks_sp,
                         u8 armor);
// Out of lives (PC game over): reset only the life counter, keep the tanks.
void player_reset_lives(void);

// Selected player 0..3 (player-select screen). Changes the sprite sheet +
// palette used in-game (each player has different weapon colours/abilities).
void player_set_number(u8 n);
u8 player_number(void);
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

// Lives (PC game_save.items.lifes): one per death, persisted across stages in
// the save slot. When they run out the caller shows GAME OVER.
u8 player_lives(void);
void player_add_life(void);              // OBJ_LIFE pickup
void player_add_tank(bool hp);           // OBJ_ENERGY_TANK / OBJ_WEAPON_TANK
void player_add_special_tank(void);      // OBJ_SPECIAL_TANK
bool player_use_tank(bool hp);           // menu: refill HP / current gauge (+special)
void player_add_armor(u8 kind);          // 0 arms, 1 body, 2 legs
u8 player_armor_ability(u8 kind);        // per-player special ability value
u8 player_armor_shot_id(void);           // ARMS charged projectile, 0xFF = none
bool player_can_shoryuken(void);         // LEGS SHORYUKEN ability
bool player_is_shoryuken(void);          // attack hitbox active
bool player_out_of_lives(void);

// pad = JOY_readJoypad(JOY_1)
void player_update(u16 pad);
// Walked into a SCROLL_LOCK column? (PC process_special_map_points)
void player_check_scroll_lock(u16 pad);

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
// starts the death sequence (player hidden, world frozen by the caller).
void player_take_damage(u8 amount);

// Death sequence: player_is_dying() while dead; player_death_update() ticks
// it and respawns at the checkpoint when lives remain, returning FALSE when
// the sequence ended (then check player_out_of_lives for game over).
bool player_is_dying(void);
bool player_death_update(void);

// Instant death, ignoring invulnerability: a platform pressing the player
// into a solid tile (PC damage(999)).
void player_crush(void);

// COIL FROG spring (PC obj_jump.start(true): 1.4x jump speed).
void player_super_jump(void);

// Record the respawn point (worldX, feet-Y worldY) when touching a
// checkpoint object. Death and pit-falls respawn there while on that map.
void player_set_checkpoint(s16 worldX, s16 worldY);

// Hide/show the player sprite (in-game menu overlay).
void player_set_visible(bool visible);

#endif
