#include "player.h"

#include "stage.h"
#include "shot.h"
#include "resources.h"

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

// Charge shot (defines.h:665 CHARGED_SHOT_INITIAL_TIME 1000ms,
// CHARGED_SHOT_TIME 2200ms at 60fps).
#define CHARGE_SEMI_FRAMES 60
#define CHARGE_FULL_FRAMES 132

// Climb pose cycle: the flip toggles every 8 climbed pixels (CLIMB_SPEED
// = 1px/frame) so the climb anim ticks along with the movement.
#define CLIMB_FLIP_PX 8

// spawn point: on the ground. Ground top is row 10 -> pixel 160
// (PC row 11; the top PC row is cropped in convert_stage.py).
#define SPAWN_X 144
#define SPAWN_Y (160 - PLAYER_BOX_B)

Player player;

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

static void make_flash_palettes(void)
{
    const u16 *base = player_sprite.palette->data;
    for (u8 v = 0; v < 2; v++) {
        u8 boost = v ? 3 : 1;
        for (u8 i = 0; i < 16; i++) {
            u16 c = base[i];
            u8 r = (c & 7) + boost;         if (r > 7) r = 7;
            u8 g = ((c >> 4) & 7) + boost;  if (g > 7) g = 7;
            u8 b = ((c >> 8) & 7) + boost;  if (b > 7) b = 7;
            flash_pal[v][i] = (b << 8) | (g << 4) | r;
        }
    }
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

// PC hit_ground ladder-edge pairs (character.cpp:1377-1412) for the fall
// scan and the standing check: a STAIR tile is ground only when the tile
// above is not STAIR (stand on the ladder top); an UNBLOCKED tile is ground
// when the tile above is STAIR (rest on the ladder's bottom rung, e.g. the
// link arrival spot at map0 (139,13)). SOLID is always ground. The other PC
// pairs are skipped on purpose: p1=UNBLOCKED with p2=SOLID would freeze the
// scan-based landing on ledges.
static bool ground_row(s16 tx, s16 row)
{
    u16 p1 = stage_terrain_at(stage_current_map, tx, row);
    if (p1 == TERRAIN_SOLID) return TRUE;
    u16 above = stage_terrain_at(stage_current_map, tx, row - 1);
    if (p1 == TERRAIN_STAIR) return above != TERRAIN_STAIR;
    if (p1 == TERRAIN_UNBLOCKED) return above == TERRAIN_STAIR;
    return FALSE;
}

void player_get_box(s16 *left, s16 *top, s16 *right, s16 *bottom)
{
    s16 ix = player.x;
    s16 iy = player.y;
    *left   = ix + PLAYER_BOX_L;
    *top    = iy + PLAYER_BOX_T;
    *right  = ix + PLAYER_BOX_R;
    *bottom = iy + PLAYER_BOX_B;
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
}

void player_init(void)
{
    player.x = SPAWN_X;
    player.y = SPAWN_Y;
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
    coyote_timer = COYOTE_FRAMES;
    jump_timer = 0;
    attack_timer = 0;
    prev_pad = 0;
    current_anim = ANIM_STAND;
    ladder_flip = FALSE;
    make_flash_palettes();

    player.spr = SPR_addSprite(&player_sprite, SPAWN_X - camera.x, SPAWN_Y - camera.y,
                               TILE_ATTR(PLAYER_PAL, FALSE, FALSE, FALSE));
    PAL_setPalette(PLAYER_PAL, player_sprite.palette->data, DMA_QUEUE);
    SPR_setAnim(player.spr, ANIM_STAND);
}

void player_update(u16 pad)
{
    s16 ix = player.x;
    s16 iy = player.y;
    s16 bl = ix + PLAYER_BOX_L;
    s16 bt = iy + PLAYER_BOX_T;
    s16 br = ix + PLAYER_BOX_R;
    s16 bb = iy + PLAYER_BOX_B;
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
            s16 nBottom = ny + PLAYER_BOX_B;
            s16 nTop    = ny + PLAYER_BOX_T;
            u16 under = stage_terrain_at(stage_current_map, ctx, nBottom >> 4);
            u16 over  = stage_terrain_at(stage_current_map, ctx, nTop >> 4);
            u16 centreNew = stage_terrain_at(stage_current_map, ctx,
                                             ((ny + PLAYER_BOX_T) + (ny + PLAYER_BOX_B)) >> 5);

            if (climb > 0 && under == TERRAIN_SOLID) {
                player.on_ladder = FALSE;                       // reached the floor
                iy = ((nBottom >> 4) << 4) - PLAYER_BOX_B;
            } else if (climb < 0 && over == TERRAIN_SOLID) {
                player.on_ladder = FALSE;                       // hit a ceiling
                iy = ((nTop >> 4) << 4) + 16 - PLAYER_BOX_T;
            } else if (centreNew != TERRAIN_STAIR &&
                       stage_terrain_at(stage_current_map, ctx, nBottom >> 4) != TERRAIN_STAIR) {
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
                s16 row = (iy + (PLAYER_BOX_T + PLAYER_BOX_B) / 2) >> 4;
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
            // Rebuild the sprite when the pose changes: the VRAM tiles are
            // (re)built on anim changes and the stairs anim is static
            // (time 0) so it never advances on its own.
            if (current_anim != ANIM_STAIRS || ladder_flip != ladder_shown) {
                current_anim = ANIM_STAIRS;
                ladder_shown = ladder_flip;
                SPR_setHFlip(player.spr, ladder_flip);
                SPR_setAnim(player.spr, ANIM_STAIRS);
            }
            SPR_setPosition(player.spr, ix - camera.x, iy - camera.y);
            prev_pad = pad;
            return;
        }
        player.vy = 0;
    }

    // ---- slide (start) -----------------------------------------------------
    // Needs a FRESH press: the PC only starts a slide after the dash button
    // was released again (_dash_button_released, character.cpp:1668), so a
    // held button cannot restart the slide when the distance limit ends it.
    bool slide_btn = (pad & SLIDE_BUTTON) != 0;
    if (!slide_btn) player.slide_prev = TRUE;
    if (!player.sliding && slide_btn && player.slide_prev && player.on_ground) {
        player.sliding = TRUE;
        player.slide_prev = FALSE;
        player.slide_dist = 0;
        player.vx = (player.facing < 0) ? -MOVE_SPEED : MOVE_SPEED;
    }

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

    if (coyote_timer > 0 && jump_timer > 0) {
        player.vy = -JUMP_VEL;
        player.on_ground = FALSE;
        coyote_timer = 0;
        jump_timer = 0;
    }
    if (jump_timer > 0) jump_timer--;

    // ---- gravity -----------------------------------------------------------
    if (!player.on_ground) {
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
        s16 t = iy + PLAYER_BOX_T;
        s16 b = iy + PLAYER_BOX_B;

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
            // falling: scan from the row the feet are in (a resting foot is
            // exactly on a tile boundary; +1 would skip that row and drop the
            // player through the floor) down to the row the feet enter.
            for (s16 row = (iy + PLAYER_BOX_B) >> 4;
                 row <= ((ny + PLAYER_BOX_B) >> 4) && !blocked; row++) {
                for (s16 tx = x0; tx <= x1; tx++) {
                    if (ground_row(tx, row)) {
                        ny = (row << 4) - PLAYER_BOX_B;
                        player.on_ground = TRUE;
                        player.vy = 0;
                        blocked = TRUE;
                        break;
                    }
                }
            }
        } else {
            // rising: scan the rows the head would cross
            for (s16 row = ((iy + PLAYER_BOX_T) >> 4) - 1;
                 row >= ((ny + PLAYER_BOX_T) >> 4) && !blocked; row--) {
                for (s16 tx = x0; tx <= x1; tx++) {
                    if (stage_terrain_at(stage_current_map, tx, row) == TERRAIN_SOLID) {
                        ny = ((row + 1) << 4) - PLAYER_BOX_T;
                        player.vy = 0;
                        blocked = TRUE;
                        break;
                    }
                }
            }
        }
        iy = ny;
    } else {
        // standing still: is there floor right under the feet?
        s16 l = ix + PLAYER_BOX_L;
        s16 r = ix + PLAYER_BOX_R;
        s16 row = (iy + PLAYER_BOX_B) >> 4;
        for (s16 tx = l >> 4; tx <= (r - 1) >> 4; tx++) {
            if (ground_row(tx, row)) {
                player.on_ground = TRUE;
                break;
            }
        }
    }

    if (player.on_ground) coyote_timer = COYOTE_FRAMES;
    else if (coyote_timer > 0) coyote_timer--;

    // ---- slide (end) -------------------------------------------------------
    // Ends on: button released, airborne, wall hit (vx zeroed by the
    // collision) or max distance (5 tiles).
    if (player.sliding) {
        s16 adx = (dx < 0) ? -dx : dx;
        player.slide_dist += adx;
        if (!(pad & SLIDE_BUTTON) || !player.on_ground ||
            player.slide_dist > SLIDE_MAX_DIST || player.vx == 0) {
            player.sliding = FALSE;
        }
    }

    player.x = ix;
    player.y = iy;

    // ---- fell out of the level ---------------------------------------------
    if (iy > STAGE0_PIXEL_H + 32) {
        player_place(SPAWN_X, SPAWN_Y);
        player.on_ground = TRUE;
        player.invuln = PLAYER_INVULN_FRAMES;
        ix = SPAWN_X;
        iy = SPAWN_Y;
        camera.x = 0;   // respawn is at the stage start; jump the camera back
        camera.y = 0;   // first so force_redraw paints the right area
        stage_force_redraw();
    }

    // ---- fire (B) + charge shot ---------------------------------------------
    // Edge-triggered like the PC (one shot per press); holding charges and
    // the release fires the semi/full charge shot (character.cpp:597).
    bool fire_raw = (pad & FIRE_BUTTON) != 0;
    bool fire_ok = fire_raw && !player.sliding;
    if (fire_ok && !player.fire_prev) {
        if (attack_timer == 0) {
            s16 sx = (player.facing > 0) ? ix + PLAYER_BOX_R : ix + PLAYER_BOX_L - 8;
            shot_fire(sx, iy + 10, player.facing);
            attack_timer = ATTACK_FRAMES;
        }
        player.charge_frames = 0;
    } else if (fire_ok) {
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
        PAL_setPalette(PLAYER_PAL, player_sprite.palette->data, DMA_QUEUE);
    }
    player.fire_prev = fire_raw;
    if (attack_timer > 0) attack_timer--;

    // ---- animation + screen position ----------------------------------------
    u16 want;
    if (!player.on_ground)      want = ANIM_JUMP;
    else if (attack_timer > 0)  want = ANIM_ATTACK;
    else if (player.sliding)    want = ANIM_SLIDE;
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

    // ---- invulnerability blink -----------------------------------------------
    if (player.invuln > 0) {
        player.invuln--;
        SPR_setVisibility(player.spr, (player.invuln & 4) ? HIDDEN : VISIBLE);
    }
}

void player_take_damage(u8 amount)
{
    if (player.invuln > 0) return;

    player.hp = (amount >= player.hp) ? 0 : player.hp - amount;
    player.invuln = PLAYER_INVULN_FRAMES;

    if (player.hp == 0) {              // death: respawn at the stage start
        player_place(SPAWN_X, SPAWN_Y);
        player.on_ground = TRUE;
        player.hp = PLAYER_INITIAL_HP;
        PAL_setPalette(PLAYER_PAL, player_sprite.palette->data, DMA_QUEUE);
        camera.x = 0;
        camera.y = 0;
        stage_force_redraw();
    }
}
