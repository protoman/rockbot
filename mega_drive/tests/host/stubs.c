/* Counting stubs for the host cost tests (tests/host).
 *
 * Every SDK/port call the code under test makes is implemented here: the sprite
 * calls and the terrain lookup bump counters, so a test can print "work per
 * frame" for the real src/object.c and assert it does not regress.
 */
#include <genesis.h>
#include <string.h>

#include "stage_data.h"
#include "player.h"
#include "object.h"

/* ---- counters the tests read ------------------------------------------ */
u32 host_terrain_calls;
u32 host_spr_calls;
u32 host_sprite_adds;

/* ---- sprite engine ---------------------------------------------------- */
static Sprite sprites[64];
static u16 sprite_used;

Sprite *SPR_addSprite(const SpriteDefinition *def, s16 x, s16 y, u16 attr)
{
    (void)attr;
    if (sprite_used >= 64) return NULL;
    Sprite *s = &sprites[sprite_used++];
    s->definition = def;
    s->x = x;
    s->y = y;
    host_sprite_adds++;
    return s;
}

void SPR_releaseSprite(Sprite *s)
{
    if (!s) return;
    s->definition = NULL;
}

void SPR_setPosition(Sprite *s, s16 x, s16 y)
{
    host_spr_calls++;
    if (s) { s->x = x; s->y = y; }
}

void SPR_setVisibility(Sprite *s, u16 v) { (void)s; (void)v; host_spr_calls++; }
void SPR_setHFlip(Sprite *s, bool f) { (void)s; (void)f; }
void SPR_setAutoAnimation(Sprite *s, bool on) { (void)s; (void)on; }
void SPR_update(void) {}

void spr_set_frame(Sprite *s, s16 frame)
{
    host_spr_calls++;
    if (s) s->frameInd = frame;
}
void spr_set_anim(Sprite *s, s16 anim)
{
    host_spr_calls++;
    if (s) s->animInd = anim;
}

/* sprite_guard resets once per sprite-engine generation */
u16 spr_gen;

/* ---- stage terrain ---------------------------------------------------- */
u8 stage_current_map;
const StageDef *stage_def;
Vect2D_s16 camera;
Map *stage_map;
const Palette *stage_bg_pal[1];

/* A flat 200x14 map with a floor on the last two rows. */
static u8 host_terrain[200 * 14];

u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY)
{
    host_terrain_calls++;
    (void)map;
    if (tileX < 0 || tileX >= 200) return TERRAIN_SOLID;
    if (tileY < 0 || tileY >= 14) return TERRAIN_UNBLOCKED;
    return host_terrain[tileY * 200 + tileX];
}

void host_terrain_fill(u8 value)
{
    memset(host_terrain, value, sizeof host_terrain);
}

void stage_horizontal_screen_move(s16 dir) { (void)dir; }
void stage_teleport_boss(u8 map, u8 x, u8 y) { (void)map; (void)x; (void)y; }

/* ---- player ----------------------------------------------------------- */
Player player;

void player_get_box(s16 *l, s16 *t, s16 *r, s16 *b)
{
    *l = (s16)(player.x + 5);
    *t = (s16)(player.y + 3);
    *r = (s16)(player.x + 21);
    *b = (s16)(player.y + 29);
}
void player_take_damage(u8 d) { (void)d; }
void player_crush(void) {}
void player_super_jump(void) {}
void player_add_life(void) {}
void player_add_tank(bool hp) { (void)hp; }
void player_add_special_tank(void) {}
void player_add_armor(u8 kind) { (void)kind; }
void player_set_checkpoint(s16 x, s16 y) { (void)x; (void)y; }
void player_use_weapon_ammo(void) {}
u8 player_weapon(void) { return 0; }
u8 player_weapon_ammo(void) { return 0; }

/* ---- enemy / sfx / dialog -------------------------------------------- */
bool enemies_subboss_alive_left(s16 tile_x) { (void)tile_x; return FALSE; }
void sfx_play(u8 id) { (void)id; }
void dialog_show_armor(u8 kind) { (void)kind; }

/* ---- SDK odds and ends ------------------------------------------------ */
void SYS_doVBlankProcess(void) {}
void VDP_setReg(u16 r, u8 v) { (void)r; (void)v; }
void kprintf(const char *fmt, ...) { (void)fmt; }
void VDP_drawText(const char *str, u16 x, u16 y) { (void)str; (void)x; (void)y; }
u16 sdk_random(void) { return 0; }
u32 getSubTick(void) { return 0; }

/* The generated object art symbols object.c references (one per def it can
 * spawn in the test): a dummy definition is enough, we never draw. */
static const SpriteDefinition dummy_def = { 0, NULL, NULL, NULL };
const SpriteDefinition object_energy_pill_big_sprite;
const SpriteDefinition object_energy_pill_small_sprite;
const SpriteDefinition object_item_jet_sprite;
const SpriteDefinition object_item_jump_sprite;
const SpriteDefinition object_weapons_pill_big_sprite;
const SpriteDefinition object_weapons_pill_small_sprite;
