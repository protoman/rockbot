#include "player.h"
#include "sfx.h"

#include "stage.h"
#include "shot.h"
#include "object.h"
#include "resources.h"
#include "player_data.h"

// Physics, tuned for 60fps. The original normalises movement by
// fps_movement_multiplier = 60/max_fps (shareddata.cpp:22); here fps is fixed.
#define MOVE_SPEED FIX16(2)
#define ACCEL      FIX16(0.35)
#define DECEL      FIX16(0.28)
#define GRAVITY    FIX16(0.25)
#define JUMP_VEL   FIX16(5.3)  // ~56px arc, about 3.5 tiles
#define MAX_FALL   FIX16(6)
#define CLIMB_SPEED FIX16(1.5)

#define COYOTE_FRAMES    6
#define JUMP_BUFFER_FRAMES 6

#define JUMP_BUTTONS BUTTON_C

// B fires (fire frame shows for the whole cooldown; ANIM_ATTACK row).
#define FIRE_BUTTON  BUTTON_B
#define ATTACK_FRAMES 12

// Slide = A on the ground (PC dash button; classplayer move() turns the
// character from input even on stairs — same here for the climb pose).
// Max distance = 5 tiles (character.cpp:1662 slide_distance > TILESIZE*5).
#define SLIDE_BUTTON   BUTTON_A
#define SLIDE_MAX_DIST 80

// PC character.cpp:24 SLIDE_Y_ADJUST — a SLIDING player with can_slide
// (ROCKBOT) drops the hitbox top by 12px (sprite_hit_area.y + 12, h - 12);
// a dash (slide_type 0) keeps the full standing box.
#define SLIDE_TOP_ADJUST 12

// Shield (PC BTN_SHIELD, classplayer.cpp:916) — ROCKBOT only. A 3-button pad
// has no free button (A=dash, B=fire, C=jump), so the gesture is DOWN+A held
// while not running; the PC requires no direction/jump for the shield anyway.
#define SHIELD_MASK (BUTTON_DOWN | BUTTON_A)

// Charge shot (defines.h:665 CHARGED_SHOT_INITIAL_TIME 1000ms,
// CHARGED_SHOT_TIME 2200ms at 60fps).
#define CHARGE_SEMI_FRAMES 60
#define CHARGE_FULL_FRAMES 132

// Hit pushback (PC character.cpp:194/3555): TILESIZE*1.2 = 19.2px away
// from the facing direction at 0.5 px/f on the ground, 0.8 in the air.
// An airborne hit cancels the jump and only pushes half the distance.
#define HIT_PUSH_FULL   FIX16(19.2)
#define HIT_PUSH_HALF   FIX16(9.6)
#define HIT_PUSH_GROUND FIX16(0.5)
#define HIT_PUSH_AIR    FIX16(0.8)

// Climb pose cycle: the flip toggles every 8 climbed pixels (CLIMB_SPEED
// = 1px/frame) so the climb anim ticks along with the movement.
#define CLIMB_FLIP_PX 8

// spawn point: on the ground. Ground top is row 10 -> pixel 160
// (PC row 11; the top PC row is cropped in convert_stage.py).
#define SPAWN_X 144
#define SPAWN_Y (160 - PLAYER_BOX_B)

Player player;

static bool air_jump_used;
static bool air_dash_used;
static u8 lives;            // PC game_save.items.lifes (5 = normal difficulty)
static bool out_of_lives;
static s16 box_t = PLAYER_BOX_T;   // hitbox offsets (see update_hitbox)
static s16 box_b = PLAYER_BOX_B;
static s16 coyote_timer;
static s16 jump_timer;
static s16 attack_timer;
static u16 prev_pad;
static u16 current_anim;
static bool ladder_flip;    // climb pose desired (cycle/turn driven)
static bool ladder_shown;   // last pose baked into the sprite VRAM
static s16 climb_flip_dist;

// charge-shot palette flash: two brighter copies of the player palette,
// strobed every 8 frames while the fire button is held (PC
// check_charging_colors cycles the armor colors the same way).
static u16 flash_pal[2][16];
static u16 base_pal[16];    // sheet palette + the current weapon's 13/14/15

// Last touched checkpoint (PC st_checkpoint: game.cpp:395 respawns there
// when set, else at the stage start). cp_y is the feet position.
static bool cp_active;
static s16 cp_map, cp_x, cp_y;

void player_set_checkpoint(s16 worldX, s16 worldY)
{
    cp_map = stage_current_map;
    cp_x = worldX;
    cp_y = worldY;
    cp_active = TRUE;
}

// Shared respawn: checkpoint on the current map, else the stage start.
// camera jumps via stage_arrival_camera so a divider stays at a screen edge.
// Feet-on-ground Y for a spawn column: the stage 0 spawn Y (160 - box) is
// CHAOS CITY's walkway; other stages have the floor a row or two off, so
// scan down for the first ground row instead of trusting a fixed Y.
static s16 spawn_ground_y(s16 x)
{
    s16 l = x + PLAYER_BOX_L, r = x + PLAYER_BOX_R;
    for (s16 row = 0; row < stage_def->map_h; row++) {
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
            u16 t = stage_terrain_at(stage_current_map, tx, row);
            if (t != TERRAIN_UNBLOCKED && t != TERRAIN_STAIR)
                return row * stage_def->tile - box_b;
        }
    }
    return SPAWN_Y;
}

static void respawn_player(bool invuln)
{
    s16 sx = SPAWN_X;
    s16 sy = spawn_ground_y(SPAWN_X);
    if (cp_active && cp_map == stage_current_map) {
        sx = cp_x;
        sy = cp_y - box_b;
    }
    player_place(sx, sy);
    player.on_ground = TRUE;
    if (invuln) player.invuln = PLAYER_INVULN_FRAMES;
    stage_arrival_camera(sx + (PLAYER_BOX_L + PLAYER_BOX_R) / 2,
                         sy + (box_t + box_b) / 2);
}

static const SpriteDefinition *const player_defs[4] = {
    &player_p1_sprite, &player_p2_sprite, &player_p3_sprite, &player_p4_sprite
};
static u8 player_n;

void player_set_number(u8 n) { player_n = (n < 4) ? n : 0; }

u8 player_max_shots(void) { return player_data[player_n].max_shots; }
u8 player_simultaneous(void) { return player_data[player_n].simultaneous; }
s8 player_damage_mod(void) { return player_data[player_n].dmg_mod; }
bool player_can_charge(void) { return (player_data[player_n].flags & PL_CHARGE) != 0; }
bool player_can_double_jump(void) { return (player_data[player_n].flags & PL_DJUMP) != 0; }
bool player_can_slide(void) { return (player_data[player_n].flags & PL_SLIDE) != 0; }
bool player_can_diagonal(void) { return (player_data[player_n].flags & PL_DIAG) != 0; }
bool player_can_air_dash(void) { return (player_data[player_n].flags & PL_AIRDASH) != 0; }
bool player_can_shield(void) { return (player_data[player_n].flags & PL_SHIELD) != 0; }
bool player_is_shielding(void) { return player.shielding; }

// Current hitbox top/bottom offsets from the sprite origin. Only a sliding
// ROCKBOT differs (PC character.cpp:2477); everything else is the base box.
static void update_hitbox(void)
{
    box_t = PLAYER_BOX_T + ((player.sliding && player_can_slide())
                            ? SLIDE_TOP_ADJUST : 0);
    box_b = PLAYER_BOX_B;
}
const SpriteDefinition *player_definition(void) { return player_defs[player_n]; }

static void make_flash_palettes(void)
{
    const u16 *base = base_pal;
    for (u8 v = 0; v < 2; v++) {
        u8 boost = v ? 3 : 1;
        for (u8 i = 0; i < 16; i++) {
            u16 c = base[i];
            u8 r = ((c >> 1) & 7) + boost;  if (r > 7) r = 7;
            u8 g = ((c >> 5) & 7) + boost;  if (g > 7) g = 7;
            u8 b = ((c >> 9) & 7) + boost;  if (b > 7) b = 7;
            flash_pal[v][i] = (u16)((r << 1) | (g << 5) | (b << 9));
        }
    }
}

static u16 rgb9(u8 r, u8 g, u8 b)
{
    // MD CRAM word: 0x0BGR — R at bits 1-3, G at 5-7, B at 9-11 (white 0x0EEE).
    return (u16)(((u8)(r >> 5) << 1) | ((u8)(g >> 5) << 5) |
                 ((u8)(b >> 5) << 9));
}

// The player palette is the sheet's, with indices 13/14/15 replaced by the
// SELECTED weapon's colours (PC classplayer.cpp change_player_color swaps the
// COLORKEY1/2/3 entries with weapon_colors[selected_weapon]). An unset colour
// keeps the sheet's baked value.
static void rebuild_player_palette(void)
{
    const u16 *sheet = player_defs[player_n]->palette->data;
    for (u8 i = 0; i < 16; i++) base_pal[i] = sheet[i];
    const s16 (*wc)[3] = player_data[player_n].wcol[player.weapon];
    for (u8 k = 0; k < 3; k++) {
        if (wc[k][0] < 0) continue;
        base_pal[13 + k] = rgb9((u8)wc[k][0], (u8)wc[k][1], (u8)wc[k][2]);
    }
    make_flash_palettes();
}

u8 player_weapon(void) { return player.weapon; }

u8 player_weapon_ammo(void) { return player.ammo[player.weapon]; }

void player_use_weapon_ammo(void)
{
    if (player.ammo[player.weapon]) player.ammo[player.weapon]--;
}

// PC classPlayer::refill_weapons() (classplayer.cpp:1078) — the game refills
// every weapon gauge at the start of each stage (game.cpp:265).
void player_refill_weapons(void)
{
    for (u8 i = 0; i < PLAYER_WEAPON_N; i++) player.ammo[i] = PLAYER_INITIAL_HP;
}

void player_set_weapon(u8 n)
{
    player.weapon = (n < PLAYER_WEAPON_N) ? n : 0;
    rebuild_player_palette();
    PAL_setPalette(PLAYER_PAL, base_pal, DMA_QUEUE);
}

static bool tile_blocks(s16 px, s16 py)
{
    return stage_terrain_at(stage_current_map, px >> 4, py >> 4) == TERRAIN_SOLID;
}

// TRUE if any tile in the column, spanning rows [t..b), is solid.
static bool column_hits_solid(s16 col, s16 t, s16 b)
{
    for (s16 ty = t >> 4; ty <= (b - 1) >> 4; ty++) {
        if (stage_terrain_at(stage_current_map, col, ty) == TERRAIN_SOLID) return TRUE;
    }
    return FALSE;
}

// PC hit_ground (character.cpp:1377-1412): feet tile vs CENTRE tile pair.
// SOLID (not UNBLOCKED/WATER/STAIR) at the feet is ground; otherwise ground
// when the feet tile differs from the centre tile (standing on a ledge edge,
// resting on a ladder's bottom rung). The pair must use the box CENTRE, not
// the tile above the feet: the map1 shaft ladder (col 198, ends at row 2
// with a 9-row pit below) releases the player with feet and centre both in
// row 3 — "above" would compare against the rung (STAIR) and freeze him in
// mid-air, while the PC centre compare lets him fall.
static bool ground_pair(s16 tx, s16 feet_row, s16 centre_row)
{
    u16 lock1 = stage_terrain_at(stage_current_map, tx, feet_row);
    if (lock1 != TERRAIN_UNBLOCKED && lock1 != TERRAIN_STAIR)
        return TRUE;
    return lock1 != stage_terrain_at(stage_current_map, tx, centre_row);
}

// A column solid across the WHOLE box height is a SIDE WALL (map border,
// divider), not floor: pressing against it mid-air must not latch
// on_ground (player_stuck_wall.png: left border at col1; the same rule
// freezes the player on the map1 ladder, boxed in by the col199 border).
// The full span matters: a 2-tile corridor has solid feet and head rows
// with open air between — that floor must still count.
static bool support_col(s16 tx, s16 top_row, s16 feet_row)
{
    for (s16 r = top_row; r <= feet_row; r++)
        if (stage_terrain_at(stage_current_map, tx, r) != TERRAIN_SOLID)
            return TRUE;                // some open row: a real floor column
    return FALSE;                       // solid for the full height: a wall
}

// Moving-platform tiles carry the player (PC check_platform_move, ~2.5px/frame
// at PC speeds; 1px/frame here) and spikes damage on contact
// (PC character::damage_spikes; PC value 188 would be instant death with the
// port's HP 28, so 4 = 2x contact damage).
#define CONVEYOR_SPEED 1
#define SPIKE_DAMAGE   4

void player_get_box(s16 *left, s16 *top, s16 *right, s16 *bottom)
{
    s16 ix = player.x;
    s16 iy = player.y;
    *left   = ix + PLAYER_BOX_L;
    *top    = iy + box_t;
    *right  = ix + PLAYER_BOX_R;
    *bottom = iy + box_b;
}

void player_place(s16 px, s16 py)
{
    player.x = px;
    player.y = py;
    player.vx = 0;
    player.vy = 0;
    player.on_ground = FALSE;
    player.on_ladder = FALSE;
    player.sliding = FALSE;
    player.slide_prev = TRUE;
    player.slide_dist = 0;
    player.charge_frames = 0;
    coyote_timer = 0;
    jump_timer = 0;
    player.hit_push = 0;
    player.hit_acc = 0;
    player.hit_dir = 0;
}

void player_init(void)
{
    player.x = SPAWN_X;
    player.y = spawn_ground_y(SPAWN_X);
    player.vx = 0;
    player.vy = 0;
    player.on_ground = TRUE;
    player.on_ladder = FALSE;
    player.facing = 1;
    player.hp = PLAYER_INITIAL_HP;
    player.invuln = 0;
    player.sliding = FALSE;
    player.slide_prev = TRUE;
    player.slide_dist = 0;
    player.charge_frames = 0;
    player.fire_prev = FALSE;
    player.hit_push = 0;
    player.hit_acc = 0;
    player.hit_dir = 0;
    player.shielding = FALSE;
    player.weapon = 0;
    player_refill_weapons();      // PC refills every gauge at stage start
    lives = 5;                    // PC reset_lifes(), normal difficulty
    out_of_lives = FALSE;
    coyote_timer = COYOTE_FRAMES;
    jump_timer = 0;
    attack_timer = 0;
    prev_pad = 0;
    current_anim = ANIM_STAND;
    ladder_flip = FALSE;
    rebuild_player_palette();

    player.spr = SPR_addSprite(player_defs[player_n], SPAWN_X - camera.x, SPAWN_Y - camera.y,
                               TILE_ATTR(PLAYER_PAL, FALSE, FALSE, FALSE));
    PAL_setPalette(PLAYER_PAL, base_pal, DMA_QUEUE);
    SPR_setAnim(player.spr, ANIM_STAND);
}

// Fire (B) + charge shot, shared by the ground and ladder paths.
// Edge-triggered like the PC (one shot per press); holding charges and
// the release fires the semi/full charge shot (character.cpp:597).
static void handle_fire(u16 pad, s16 ix, s16 iy)
{
    // PC attack() returns early while the shield is up (classplayer.cpp:300).
    // fire_prev still tracks the real button so releasing B mid-shield does
    // not fire a stored charge shot.
    if (player.shielding) {
        player.fire_prev = (pad & FIRE_BUTTON) != 0;
        if (attack_timer > 0) attack_timer--;
        return;
    }

    bool fire_raw = (pad & FIRE_BUTTON) != 0;
    bool fire_ok = fire_raw && !player.sliding;
    if (fire_ok && !player.fire_prev) {
        if (attack_timer == 0) {
            s16 sx = (player.facing > 0) ? ix + PLAYER_BOX_R : ix + PLAYER_BOX_L - 8;
            // PC can_shot_diagonal (P2): Up/Down held aims the shot (proj
            // 12/13 diagonal up/down, same art).
            s16 dy = 0;
            if (player_can_diagonal()) {
                if (pad & BUTTON_UP) dy = -3;
                else if (pad & BUTTON_DOWN) dy = 3;
            }
            shot_fire(sx, iy + 10, player.facing, dy);
            attack_timer = ATTACK_FRAMES;
        }
        player.charge_frames = 0;
    } else if (fire_ok && player_can_charge()) {
        if (player.charge_frames < 60000) player.charge_frames++;
        if ((player.charge_frames & 7) == 0) {
            PAL_setPalette(PLAYER_PAL,
                           flash_pal[(player.charge_frames >> 3) & 1], DMA_QUEUE);
        }
    } else if (player.fire_prev && !fire_raw) {
        if (!player.sliding) {
            s16 sx = (player.facing > 0) ? ix + PLAYER_BOX_R - 4 : ix + PLAYER_BOX_L - 12;
            if (player.charge_frames >= CHARGE_FULL_FRAMES) {
                shot_charge_fire(sx, iy + 8, player.facing, 2);
                attack_timer = ATTACK_FRAMES;
            } else if (player.charge_frames >= CHARGE_SEMI_FRAMES) {
                shot_charge_fire(sx, iy + 8, player.facing, 1);
                attack_timer = ATTACK_FRAMES;
            }
        }
        player.charge_frames = 0;
        PAL_setPalette(PLAYER_PAL, base_pal, DMA_QUEUE);
    }
    player.fire_prev = fire_raw;
    if (attack_timer > 0) attack_timer--;
}

// Damage blink. Must run on the ladder path too: that path returns before
// the end of player_update, so a player who grabbed a ladder mid-blink kept
// whatever visibility the last blink frame set (often hidden for the whole
// climb).
static void invuln_blink(void)
{
    if (player.invuln > 0) {
        player.invuln--;
        SPR_setVisibility(player.spr, (player.invuln & 4) ? HIDDEN : VISIBLE);
    }
}

void player_update(u16 pad)
{
    update_hitbox();   // box_t/box_b from the slide state so far
    s16 ix = player.x;
    s16 iy = player.y;
    s16 bl = ix + PLAYER_BOX_L;
    s16 bt = iy + box_t;
    s16 br = ix + PLAYER_BOX_R;
    s16 bb = iy + box_b;
    s16 ctx = (bl + br) >> 5;   // centre tile X  ((bl+br)/2) >> 4
    s16 cty = (bt + bb) >> 5;   // centre tile Y

    // ---- ladder ------------------------------------------------------------
    // Grab clears on_ground so gravity cannot re-latch while climbing.
    // PC has no sideways let-go: horizontal input is gated while the ladder
    // anim plays (character.cpp:218/266), which is also what lets the
    // per-frame X centring below stick (the old same-frame let-go fired
    // before player.x = ix ran, leaving the player beside the stair).
    if (!player.on_ladder) {
        if ((pad & BUTTON_UP) && stage_terrain_at(stage_current_map, ctx, cty) == TERRAIN_STAIR) {
            player.on_ladder = TRUE;
            player.on_ground = FALSE;
            ix = (ctx << 4) + 8 - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;   // centre on ladder (character.cpp:396)
        } else if ((pad & BUTTON_DOWN) &&
                   stage_terrain_at(stage_current_map, ctx, bb >> 4) == TERRAIN_STAIR) {
            player.on_ladder = TRUE;
            player.on_ground = FALSE;
            ix = (ctx << 4) + 8 - (PLAYER_BOX_L + PLAYER_BOX_R) / 2;
        }
    }

    // freshly grabbed: pose starts from the approach direction, cycle restarts
    if (player.on_ladder && current_anim != ANIM_STAIRS) {
        ladder_flip = (player.facing < 0);
        climb_flip_dist = 0;
    }

    if (player.on_ladder) {
        player.vx = 0;
        // PC slide() bails out in a stairs frame (character.cpp:1598): a ladder
        // grab cancels a slide, and with it the short slide hitbox.
        player.sliding = FALSE;
        update_hitbox();
        // PC turns the climb sprite in place from left/right
        // (character.cpp:266-269) without moving off the ladder.
        if ((pad & BUTTON_RIGHT) && player.facing != 1) {
            player.facing = 1;
            ladder_flip = FALSE;
        } else if ((pad & BUTTON_LEFT) && player.facing != -1) {
            player.facing = -1;
            ladder_flip = TRUE;
        }
        s16 climb = 0;
        if (pad & BUTTON_UP)   climb = -1;
        if (pad & BUTTON_DOWN) climb =  1;

        if (climb != 0) {
            s16 ny = iy + climb * F16_toInt(CLIMB_SPEED);
            s16 nBottom = ny + box_b;
            s16 nTop    = ny + box_t;
            u16 under = stage_terrain_at(stage_current_map, ctx, nBottom >> 4);
            u16 over  = stage_terrain_at(stage_current_map, ctx, nTop >> 4);
            u16 centreNew = stage_terrain_at(stage_current_map, ctx,
                                             ((ny + box_t) + (ny + box_b)) >> 5);

            if (climb > 0 && under == TERRAIN_SOLID) {
                player.on_ladder = FALSE;                       // reached the floor
                iy = ((nBottom >> 4) << 4) - box_b;
            } else if (climb < 0 && over == TERRAIN_SOLID) {
                player.on_ladder = FALSE;                       // hit a ceiling
                iy = ((nTop >> 4) << 4) + 16 - box_t;
            } else if (centreNew != TERRAIN_STAIR &&
                       stage_terrain_at(stage_current_map, ctx, nBottom >> 4) != TERRAIN_STAIR) {
                // Apply the pending step before releasing: skipping it left
                // the player one pixel high — centre still in the stair row,
                // feet already below — and the feet-vs-centre ground pair
                // then latched him in mid-air wherever a ladder ends above
                // open space (map1 col 1). With iy = ny both rows are off
                // the stair, the pair sees no ground and he falls.
                iy = ny;
                player.on_ladder = FALSE;                       // walked off the ladder
            } else {
                iy = ny;
                // climb step: toggle the pose every CLIMB_FLIP_PX pixels
                if (++climb_flip_dist >= CLIMB_FLIP_PX) {
                    climb_flip_dist = 0;
                    ladder_flip = !ladder_flip;
                }
            }

            if (!player.on_ladder) {
                // A box wider than one tile can overlap a side wall when
                // centred on the ladder column (e.g. the right-edge divider
                // at map1 col 199). The feet-row ground check would then
                // latch on the wall and freeze the player mid-air. Shift X
                // clear of the wall so the fall scan misses it.
                s16 row = (iy + (box_t + box_b) / 2) >> 4;
                s16 rc = (ix + PLAYER_BOX_R - 1) >> 4;
                if (stage_terrain_at(stage_current_map, rc, row) == TERRAIN_SOLID)
                    ix = (rc << 4) - PLAYER_BOX_R;
                s16 lc = (ix + PLAYER_BOX_L) >> 4;
                if (stage_terrain_at(stage_current_map, lc, row) == TERRAIN_SOLID)
                    ix = ((lc + 1) << 4) - PLAYER_BOX_L;
            }
        }

        if (player.on_ladder) {
            player.x = ix;
            player.y = iy;
            handle_fire(pad, ix, iy);   // can shoot while climbing (PC)
            // PC keeps the stairs pose; firing mid-climb uses
            // ANIM_TYPE_STAIRS_ATTACK (classplayer.cpp:482).
            u16 stair_anim = (attack_timer > 0) ? ANIM_STAIRS_ATTACK : ANIM_STAIRS;
            // Rebuild the sprite when the pose changes: the VRAM tiles are
            // (re)built on anim changes and both rows are static (time 0)
            // so they never advance on their own.
            if (current_anim != stair_anim || ladder_flip != ladder_shown) {
                current_anim = stair_anim;
                ladder_shown = ladder_flip;
                SPR_setHFlip(player.spr, ladder_flip);
                SPR_setAnim(player.spr, stair_anim);
            }
            SPR_setPosition(player.spr, ix - camera.x, iy - camera.y);
            invuln_blink();
            // JUMP press on a ladder = let go and fall in place (the press
            // is consumed so it cannot double as a ground jump).
            if ((pad & JUMP_BUTTONS) && !(prev_pad & JUMP_BUTTONS)) {
                player.on_ladder = FALSE;
                player.vy = 0;
                prev_pad = pad;
            } else {
                prev_pad = pad;
                return;
            }
        }
        player.vy = 0;
    }

    // ---- hit pushback (PC ANIM_TYPE_HIT, character.cpp:191-215) -----------
    // While pushing back, movement/jump/slide input is ignored and the
    // player slides away from the direction he was facing.
    if (player.hit_push > 0) {
        fix16 speed = player.on_ground ? HIT_PUSH_GROUND : HIT_PUSH_AIR;
        player.hit_push -= speed;
        player.hit_acc += speed;
        s16 mv = F16_toInt(player.hit_acc);
        player.hit_acc -= FIX16(mv);
        if (mv > 0) {
            s16 nx = ix + player.hit_dir * mv;
            s16 t = iy + box_t;
            s16 b = iy + box_b;
            if (player.hit_dir > 0) {
                s16 col = (nx + PLAYER_BOX_R - 1) >> 4;
                if (column_hits_solid(col, t, b)) nx = (col << 4) - PLAYER_BOX_R;
            } else {
                s16 col = (nx + PLAYER_BOX_L) >> 4;
                if (column_hits_solid(col, t, b)) nx = ((col + 1) << 4) - PLAYER_BOX_L;
            }
            ix = nx;
        }
        player.vx = 0;
        pad &= ~(BUTTON_LEFT | BUTTON_RIGHT | JUMP_BUTTONS | SLIDE_BUTTON);
    }

    // ---- shield (PC classplayer.cpp:916, ROCKBOT only) ---------------------
    // have_shield && no direction/jump && shield held -> ANIM_TYPE_SHIELD; the
    // player can't attack (classplayer.cpp:300) and enemy shots are blocked.
    player.shielding = FALSE;
    if (player.hit_push <= 0 && player_can_shield() &&
        (pad & SHIELD_MASK) == SHIELD_MASK &&
        !(pad & (BUTTON_LEFT | BUTTON_RIGHT | JUMP_BUTTONS))) {
        player.shielding = TRUE;
        // PC zeroes the directions while shielding (classplayer.cpp:842) and
        // attack() returns early, so the whole pad is consumed here (this also
        // keeps A from starting a slide and C from jumping).
        pad &= ~(BUTTON_LEFT | BUTTON_RIGHT | BUTTON_UP | BUTTON_DOWN |
                 JUMP_BUTTONS | FIRE_BUTTON | SLIDE_BUTTON);
    }

    // ---- slide / dash (start) ----------------------------------------------
    // Needs a FRESH press: the PC only starts a slide after the dash button
    // was released again (_dash_button_released, character.cpp:1668), so a
    // held button cannot restart the slide when the distance limit ends it.
    bool slide_btn = (pad & SLIDE_BUTTON) != 0;
    if (!slide_btn) player.slide_prev = TRUE;
    // PC dash (can_slide=false) is the same button + anim; the only difference
    // is the hitbox height, handled by update_hitbox() -> allow all players.
    if (!player.sliding && slide_btn && player.slide_prev) {
        if (player.on_ground) {
            player.sliding = TRUE;
            player.slide_prev = FALSE;
            player.slide_dist = 0;
            player.vx = (player.facing < 0) ? -MOVE_SPEED : MOVE_SPEED;
        } else if (player_can_air_dash() && !air_dash_used) {
            // PC air dash (character.cpp:1668): the SAME ANIM_TYPE_SLIDE state,
            // gravity off, up to 5 tiles, once per airtime (_can_execute_airdash).
            air_dash_used = TRUE;
            player.sliding = TRUE;
            player.slide_prev = FALSE;
            player.slide_dist = 0;
            player.vy = 0;   // PC _obj_jump.finish(): the dash is horizontal
            player.vx = (player.facing < 0) ? -MOVE_SPEED : MOVE_SPEED;
        }
    }

    // The slide may have just started: refresh the box before it is used.
    update_hitbox();

    // ---- horizontal --------------------------------------------------------
    if (player.sliding) {
        // PC lets left/right turn the character even mid-slide
        // (character.cpp:266 includes ANIM_TYPE_SLIDE) — the slide follows.
        if ((pad & BUTTON_RIGHT) && player.facing != 1) {
            player.facing = 1;
        } else if ((pad & BUTTON_LEFT) && player.facing != -1) {
            player.facing = -1;
        }
        player.vx = (player.facing < 0) ? -MOVE_SPEED : MOVE_SPEED;
    } else if (pad & BUTTON_RIGHT) {
        player.facing = 1;
        player.vx += ACCEL;
        if (player.vx > MOVE_SPEED) player.vx = MOVE_SPEED;
    } else if (pad & BUTTON_LEFT) {
        player.facing = -1;
        player.vx -= ACCEL;
        if (player.vx < -MOVE_SPEED) player.vx = -MOVE_SPEED;
    } else if (player.vx > 0) {
        player.vx -= DECEL;
        if (player.vx < 0) player.vx = 0;
    } else if (player.vx < 0) {
        player.vx += DECEL;
        if (player.vx > 0) player.vx = 0;
    }

    // ---- jump (edge triggered, buffered + coyote time) ----------------------
    if ((pad & JUMP_BUTTONS) && !(prev_pad & JUMP_BUTTONS)) {
        jump_timer = JUMP_BUFFER_FRAMES;
    }
    if (!(pad & JUMP_BUTTONS) && player.vy < 0) {
        player.vy = F16_mul(player.vy, FIX16(0.5));   // variable jump height
    }
    prev_pad = pad;

    if (player.on_ground) { air_jump_used = FALSE; air_dash_used = FALSE; }
    // PC blocks jumping mid air-dash (character.cpp:1761).
    if (player.sliding && !player.on_ground) jump_timer = 0;
    if (coyote_timer > 0 && jump_timer > 0) {
        sfx_play(SFX_JUMP);
        player.vy = -JUMP_VEL;
        player.on_ground = FALSE;
        coyote_timer = 0;
        jump_timer = 0;
    } else if (jump_timer > 0 && !player.on_ground &&
               player_can_double_jump() && !air_jump_used) {
        // PC can_double_jump (P2 CANDY): one extra jump while airborne.
        sfx_play(SFX_JUMP);
        player.vy = -JUMP_VEL;
        air_jump_used = TRUE;
        jump_timer = 0;
    }
    if (jump_timer > 0) jump_timer--;

    // ---- gravity -----------------------------------------------------------
    // PC skips gravity while the dash runs (character.cpp:1201: an air-dash
    // player stays in ANIM_TYPE_SLIDE, so the dash is a flat 5-tile move).
    if (!player.on_ground && !player.sliding) {
        if (player.vy < MAX_FALL) player.vy += GRAVITY;
        if (player.vy > MAX_FALL) player.vy = MAX_FALL;
    }

    s16 dx = F16_toInt(player.vx);
    s16 dy = F16_toInt(player.vy);

    // ---- horizontal move + resolve -----------------------------------------
    // Test only the column the leading edge enters and snap flush against its
    // face. The old whole-box test snapped by the rightmost/leftmost column
    // even when the overlap was in the far one, yanking the player up to a
    // full box-width backward ("bounce back"). See findings.md FIND-09.
    if (dx != 0) {
        s16 nx = ix + dx;
        s16 t = iy + box_t;
        s16 b = iy + box_b;

        if (dx > 0) {
            s16 col = (nx + PLAYER_BOX_R - 1) >> 4;
            if (column_hits_solid(col, t, b)) {
                nx = (col << 4) - PLAYER_BOX_R;   // exact edge contact
                player.vx = 0;
            }
        } else {
            s16 col = (nx + PLAYER_BOX_L) >> 4;
            if (column_hits_solid(col, t, b)) {
                nx = ((col + 1) << 4) - PLAYER_BOX_L;
                player.vx = 0;
            }
        }
        // platform bodies are solid from the side too (PC COLlISION_MODE_XY,
        // classmap.cpp:1433). Strict overlap: edge contact while standing on
        // top (b == o->y) never hits, so walking along the top stays free.
        s16 px;
        if (objects_block_x(nx, t, b, dx, &px)) {
            nx = px;
            player.vx = 0;
        }
        ix = nx;
    }

    // ---- vertical move + resolve -------------------------------------------
    // Only the row the feet enter (falling) or the row the head enters (rising)
    // may block. Testing the whole box here would treat a side-hit against a
    // step as "landed": on_ground would latch, gravity would stop and the
    // player would freeze mid-air. See lessons_learned.md #23.
    player.on_ground = FALSE;

    if (dy != 0) {
        s16 ny = iy + dy;
        s16 l = ix + PLAYER_BOX_L;
        s16 r = ix + PLAYER_BOX_R;
        s16 x0 = l >> 4;
        s16 x1 = (r - 1) >> 4;
        bool blocked = FALSE;

        if (dy > 0) {
            s16 plat_top = objects_land_check(l, r,
                                              ny + box_t,
                                              ny + box_b);
            // falling: scan from the row the feet are in (a resting foot is
            // exactly on a tile boundary; +1 would skip that row and drop the
            // player through the floor) down to the row the feet enter. The
            // current (unsnapped) pose keeps its real centre row; deeper
            // candidate rows use the snapped-pose centre (row - 1).
            s16 centre_now = (iy + (box_t + box_b) / 2) >> 4;
            s16 top_now = (iy + box_t) >> 4;
            for (s16 row = (iy + box_b) >> 4;
                 row <= ((ny + box_b) >> 4) && !blocked; row++) {
                s16 centre_row = (row == ((iy + box_b) >> 4))
                                     ? centre_now : row - 1;
                for (s16 tx = x0; tx <= x1; tx++) {
                    if (ground_pair(tx, row, centre_row) &&
                        support_col(tx, top_now, row)) {
                        ny = (row << 4) - box_b;
                        player.on_ground = TRUE;
                        player.vy = 0;
                        blocked = TRUE;
                        break;
                    }
                }
            }
            // one-way platform catch: wins against the terrain landing; ties
            // go to the platform so a flush spawn still activates
            if (plat_top >= 0 && (!blocked || plat_top <= ny + box_b)) {
                ny = plat_top - box_b;
                player.on_ground = TRUE;
                player.vy = 0;
                objects_player_land(l, r, plat_top);
            }
        } else {
            // rising: scan the rows the head would cross
            for (s16 row = ((iy + box_t) >> 4) - 1;
                 row >= ((ny + box_t) >> 4) && !blocked; row--) {
                for (s16 tx = x0; tx <= x1; tx++) {
                    if (stage_terrain_at(stage_current_map, tx, row) == TERRAIN_SOLID) {
                        ny = ((row + 1) << 4) - box_t;
                        player.vy = 0;
                        blocked = TRUE;
                        break;
                    }
                }
            }
            // platform bottoms are solid too (PC XY mode); the tighter
            // (lower) of terrain vs platform wins
            s16 pny = objects_head_block(l, r, ny + box_t,
                                         iy + box_b);
            if (pny > ny) {
                ny = pny;
                player.vy = 0;
            }
        }
        iy = ny;
    } else {
        // standing still: is there floor right under the feet?
        s16 l = ix + PLAYER_BOX_L;
        s16 r = ix + PLAYER_BOX_R;
        s16 row = (iy + box_b) >> 4;
        s16 centre_row = (iy + (box_t + box_b) / 2) >> 4;
        s16 top_row = (iy + box_t) >> 4;
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
            if (ground_pair(tx, row, centre_row) &&
                support_col(tx, top_row, row)) {
                player.on_ground = TRUE;
                break;
            }
        }
        // or a platform top at foot level (keeps the player "grounded" while
        // a fly platform carries him upward over empty space)
        if (!player.on_ground &&
            objects_stand_check(l, r, iy + box_b))
            player.on_ground = TRUE;
    }

    if (player.on_ground) coyote_timer = COYOTE_FRAMES;
    else if (coyote_timer > 0) coyote_timer--;

    // ---- slide (end) -------------------------------------------------------
    // Ends on: button released, wall hit (vx zeroed by the collision) or max
    // distance (5 tiles). Air-dash players keep sliding off a ledge — PC only
    // ends on no-ground when can_air_dash() is false (character.cpp:1694).
    if (player.sliding) {
        s16 adx = (dx < 0) ? -dx : dx;
        player.slide_dist += adx;
        bool no_ground_ends = !player_can_air_dash();
        if (!(pad & SLIDE_BUTTON) ||
            (no_ground_ends && !player.on_ground) ||
            player.slide_dist > SLIDE_MAX_DIST || player.vx == 0) {
            player.sliding = FALSE;
        }
    }

    // ---- moving-platform tiles (PC check_platform_move) --------------------
    if (player.on_ground && !player.on_ladder) {
        s16 fcx = (ix + (PLAYER_BOX_L + PLAYER_BOX_R) / 2) >> 4;
        s16 fcy = (iy + box_b) >> 4;
        u16 ft = stage_terrain_at(stage_current_map, fcx, fcy);
        if (ft == TERRAIN_MOVE_LEFT || ft == TERRAIN_MOVE_RIGHT) {
            s16 nx = ix + ((ft == TERRAIN_MOVE_LEFT) ? -CONVEYOR_SPEED : CONVEYOR_SPEED);
            s16 t = iy + box_t, b = iy + box_b;
            s16 col = (ft == TERRAIN_MOVE_LEFT) ? ((nx + PLAYER_BOX_L) >> 4)
                                                : ((nx + PLAYER_BOX_R - 1) >> 4);
            if (!column_hits_solid(col, t, b)) ix = nx;
        }
    }

    // ---- spikes (PC damage_spikes) -----------------------------------------
    {
        s16 l = ix + PLAYER_BOX_L, r = ix + PLAYER_BOX_R;
        s16 t = iy + box_t, b = iy + box_b;
        bool spiked = FALSE;
        for (s16 ty = t >> 4; ty <= (b - 1) >> 4 && !spiked; ty++) {
            for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
                if (stage_terrain_at(stage_current_map, tx, ty) == TERRAIN_SPIKE) {
                    spiked = TRUE;
                    break;
                }
            }
        }
        if (spiked) player_take_damage(SPIKE_DAMAGE);
    }

    player.x = ix;
    player.y = iy;

    // ---- fell out of the level ---------------------------------------------
    if (iy > (stage_def->map_h * stage_def->tile) + 32) {
        respawn_player(TRUE);
        ix = player.x;
        iy = player.y;
    }

    // ---- fire (B) + charge shot ---------------------------------------------
    handle_fire(pad, ix, iy);

    // ---- animation + screen position ----------------------------------------
    u16 want;
    if (player.hit_push > 0)    want = ANIM_HIT;
    else if (player.shielding)  want = ANIM_SHIELD;
    else if (player.sliding)    want = ANIM_SLIDE;   // ground slide + air dash
    else if (!player.on_ground) want = ANIM_JUMP;
    else if (attack_timer > 0)  want = ANIM_ATTACK;
    else if (player.vx != 0)    want = ANIM_WALK;
    else                        want = ANIM_STAND;

    // Flip before setAnim: the VRAM tiles are (re)built on anim changes, so
    // a flip set after a static/single-frame anim switch never reaches VRAM.
    SPR_setHFlip(player.spr, player.facing < 0);
    if (want != current_anim) {
        current_anim = want;
        SPR_setAnim(player.spr, want);
    }
    SPR_setPosition(player.spr, ix - camera.x, iy - camera.y);

    invuln_blink();
}

void player_set_visible(bool visible)
{
    if (player.spr) SPR_setVisibility(player.spr, visible ? VISIBLE : HIDDEN);
}

// PC classPlayer::death() (classplayer.cpp:951): one life per death; running
// out is a game over.
static void player_die(void)
{
    if (lives) lives--;
    else out_of_lives = TRUE;
    player.hp = PLAYER_INITIAL_HP;
    PAL_setPalette(PLAYER_PAL, base_pal, DMA_QUEUE);
    respawn_player(FALSE);
}

void player_take_damage(u8 amount)
{
    if (player.invuln > 0) return;

    // PC character.cpp:3001-3004: incoming damage is clamped to >=1, then the
    // player's damage_modifier lands only if the result stays > 0. It is a
    // DEFENSIVE trait (set on the character RECEIVING damage): BETABOT -1
    // (tougher), CANDYBOT +1 (fragile) — it does not touch the player's shots.
    s16 dmg = (s16)amount;
    if (dmg < 1) dmg = 1;
    if (dmg + player_damage_mod() > 0) dmg += player_damage_mod();

    player.hp = (dmg >= (s16)player.hp) ? 0 : (u8)(player.hp - dmg);
    player.invuln = PLAYER_INVULN_FRAMES;
    sfx_play((player.hp == 0) ? SFX_PLAYER_DEATH : SFX_PLAYER_HIT);

    // PC ANIM_TYPE_HIT replaces ANIM_TYPE_SLIDE (character.cpp:1598), so a hit
    // cancels a slide/air dash — otherwise the dash's gravity-off would leave
    // the player floating through the hit pushback.
    player.sliding = FALSE;
    player.slide_dist = 0;

    // Enter the hit state (PC character.cpp:3036): push away from facing,
    // half distance + jump cancelled when hit in the air.
    if (player.hit_push <= 0) {
        player.hit_dir = (player.facing > 0) ? -1 : 1;
        player.hit_acc = 0;
        if (player.on_ground) {
            player.hit_push = HIT_PUSH_FULL;
        } else {
            player.hit_push = HIT_PUSH_HALF;
            if (player.vy < 0) player.vy = 0;   // _obj_jump.finish()
        }
    }

    if (player.hp == 0) player_die();
}

void player_crush(void)
{
    // A platform pressing the player into a solid tile is instant death
    // (PC damage(999), classmap.cpp:1644). Bypasses the invulnerability
    // window — being crushed is not a "hit".
    player.hp = 0;
    player_die();
}

u8 player_lives(void) { return lives; }

bool player_out_of_lives(void) { return out_of_lives; }
