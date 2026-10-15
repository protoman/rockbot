#include "player.h"

#include "stage.h"
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

#define JUMP_BUTTONS (BUTTON_A | BUTTON_B | BUTTON_C)

// spawn point: on the ground. Ground top is row 11 -> pixel 176.
#define SPAWN_X 144
#define SPAWN_Y (176 - PLAYER_BOX_B)

Player player;

static s16 coyote_timer;
static s16 jump_timer;
static u16 prev_pad;
static u16 current_anim;

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
    coyote_timer = COYOTE_FRAMES;
    jump_timer = 0;
    prev_pad = 0;
    current_anim = ANIM_STAND;

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
    if (!player.on_ladder) {
        if ((pad & BUTTON_UP) && stage_terrain_at(stage_current_map, ctx, cty) == TERRAIN_STAIR) {
            player.on_ladder = TRUE;
        } else if ((pad & BUTTON_DOWN) &&
                   stage_terrain_at(stage_current_map, ctx, bb >> 4) == TERRAIN_STAIR) {
            player.on_ladder = TRUE;
        }
    }

    if (player.on_ladder) {
        if ((pad & (BUTTON_LEFT | BUTTON_RIGHT)) && !player.on_ground) {
            player.on_ladder = FALSE;   // let go sideways
        } else {
            player.vx = 0;
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
                } else if (centreNew != TERRAIN_STAIR) {
                    player.on_ladder = FALSE;                       // walked off the ladder
                } else {
                    iy = ny;
                }
            }

            if (player.on_ladder) {
                player.x = ix;
                player.y = iy;
                if (current_anim != ANIM_STAIRS) {
                    current_anim = ANIM_STAIRS;
                    SPR_setAnim(player.spr, ANIM_STAIRS);
                }
                SPR_setHFlip(player.spr, player.facing < 0);
                SPR_setPosition(player.spr, ix - camera.x, iy - camera.y);
                prev_pad = pad;
                return;
            }
            player.vy = 0;
        }
    }

    // ---- horizontal --------------------------------------------------------
    if (pad & BUTTON_RIGHT) {
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
                    if (stage_terrain_at(stage_current_map, tx, row) == TERRAIN_SOLID) {
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
            if (stage_terrain_at(stage_current_map, tx, row) == TERRAIN_SOLID) {
                player.on_ground = TRUE;
                break;
            }
        }
    }

    if (player.on_ground) coyote_timer = COYOTE_FRAMES;
    else if (coyote_timer > 0) coyote_timer--;

    player.x = ix;
    player.y = iy;

    // ---- fell out of the level ---------------------------------------------
    if (iy > STAGE0_PIXEL_H + 32) {
        player_place(SPAWN_X, SPAWN_Y);
        player.on_ground = TRUE;
        ix = SPAWN_X;
        iy = SPAWN_Y;
        stage_force_redraw();   // camera jumps back; force a full MAP refresh
    }

    // ---- animation + screen position ----------------------------------------
    u16 want;
    if (!player.on_ground)      want = ANIM_JUMP;
    else if (player.vx != 0)    want = ANIM_WALK;
    else                        want = ANIM_STAND;

    if (want != current_anim) {
        current_anim = want;
        SPR_setAnim(player.spr, want);
    }

    SPR_setHFlip(player.spr, player.facing < 0);
    SPR_setPosition(player.spr, ix - camera.x, iy - camera.y);
}
