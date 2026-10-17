#include "hud.h"

#include "enemy.h"
#include "player.h"
#include "resources.h"

// Player + boss HP as energy balls (PC draw::draw_enery_ball, draw.cpp:1272):
// 5 sprites of 16x16 at the top-left (3,3) and the boss row at RES_W-55.
// The sheets come from tools/convert_assets.py quantized with the shared
// PAL0 (tools/pal_ui.py) so the fill colors match the enemy palette line.
#define HUD_BALLS 5
#define HUD_BALL_X 3
#define HUD_BOSS_X (320 - 55)  // PC draw.cpp:1237
#define HUD_BALL_Y 3
#define HUD_BALL_STEP 10

static Sprite *balls[HUD_BALLS];
static s8 last_fill[HUD_BALLS];
static Sprite *boss_balls[HUD_BALLS];
static s8 boss_last_fill[HUD_BALLS];
static bool boss_row_visible;
static bool hud_on = TRUE;

// Ball frame for a hp percentage: 4 x 5% slices per ball (PC fill steps).
static s8 ball_fill(s16 pct, u8 n)
{
    s16 t = pct - 20 * n;   // slice per ball: 20% (4 x 5%)
    return (t >= 20) ? 0 : (t >= 15) ? 1 : (t >= 10) ? 2 : (t >= 5) ? 3 : 4;
}

void hud_init(void)
{
    for (u8 i = 0; i < HUD_BALLS; i++) {
        balls[i] = SPR_addSprite(&hud_hp_ball_sprite,
                                 HUD_BALL_X + HUD_BALL_STEP * i, HUD_BALL_Y,
                                 TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        last_fill[i] = -1;
        boss_balls[i] = SPR_addSprite(&hud_boss_ball_sprite,
                                      HUD_BOSS_X + HUD_BALL_STEP * i, HUD_BALL_Y,
                                      TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
        boss_last_fill[i] = -1;
        if (boss_balls[i]) SPR_setVisibility(boss_balls[i], HIDDEN);
    }
    boss_row_visible = FALSE;
    hud_on = TRUE;
}

void hud_update(u8 hp)
{
    if (!hud_on) return;
    s16 pct = (100 * hp) / PLAYER_INITIAL_HP;
    for (u8 i = 0; i < HUD_BALLS; i++) {
        s8 fill = ball_fill(pct, i);
        if (fill != last_fill[i]) {
            last_fill[i] = fill;
            if (balls[i]) SPR_setFrame(balls[i], fill);
        }
    }
}

void hud_boss(s16 hp)
{
    if (!hud_on) return;
    bool show = (hp > 0);
    if (show != boss_row_visible) {
        boss_row_visible = show;
        for (u8 i = 0; i < HUD_BALLS; i++) {
            if (boss_balls[i]) SPR_setVisibility(boss_balls[i], show ? VISIBLE : HIDDEN);
            boss_last_fill[i] = -1;   // force a frame refresh on re-show
        }
    }
    if (!show) return;
    s16 pct = (100 * hp) / BOSS_INITIAL_HP;
    for (u8 i = 0; i < HUD_BALLS; i++) {
        s8 fill = ball_fill(pct, i);
        if (fill != boss_last_fill[i]) {
            boss_last_fill[i] = fill;
            if (boss_balls[i]) SPR_setFrame(boss_balls[i], fill);
        }
    }
}

void hud_set_visible(bool visible)
{
    hud_on = visible;
    for (u8 i = 0; i < HUD_BALLS; i++) {
        if (balls[i]) SPR_setVisibility(balls[i], visible ? VISIBLE : HIDDEN);
        if (boss_balls[i])
            SPR_setVisibility(boss_balls[i],
                              (visible && boss_row_visible) ? VISIBLE : HIDDEN);
    }
}
