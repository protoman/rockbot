#include "config.h"
#include "tdump.h"
#include "pal_safe.h"
#include "music.h"
#include "resources.h"

#include <genesis.h>
#include "sram_safe.h"

#include "sfx.h"
#include "ending.h"
#include "stage.h"   // SPR_* fixed tile blocks below the sprite pool

// SRAM: 5 save slots occupy bytes 8..47; options live at byte 48.
#define CFG_OFF   48
// Music volume byte (+9): bit 7 says "the player's choice was stored". Configs saved
// before the option existed hold 0 there, which used to load as volume 0 = silent.
#define CFG_VOL_SET 0x80
#define CFG_MAGIC 0x4F50       // 'OP'

Config config;

static const char *const sfx_names[SFX_COUNT] = {
    "SHOT", "CHARGED SHOT", "JUMP", "PLAYER HIT", "PLAYER DEATH",
    "CHARGING 1", "CHARGING 2", "ENEMY HIT", "ENEMY KILLED", "EXPLOSION",
    "ENERGY PILL", "ITEM", "TELEPORT", "WATER ENTER", "WATER LEAVE",
    "DOOR OPEN", "GOT WEAPON", "STAGE SELECT", "CURSOR",
};

void config_init(void)
{
    if (SRAM_readWord(CFG_OFF) != CFG_MAGIC) {
        config.sound = 1;
        config.music_vol = 3;
        config.turbo = 0;
        config.autocharge = 0;
        config.b_jump = 2;     // C
        config.b_fire = 1;     // B
        config.b_dash = 0;     // A
        config.game_finished = 0;
        config_save();
        sfx_set_enabled(1);
        return;
    }
    config.sound = SRAM_readByte(CFG_OFF + 2);
    {
        const u8 v = SRAM_readByte(CFG_OFF + 9);
        config.music_vol = (v & CFG_VOL_SET) ? (u8)(v & 3) : 3;   // unset -> default
    }
    config.turbo = SRAM_readByte(CFG_OFF + 3);
    config.autocharge = SRAM_readByte(CFG_OFF + 4);
    config.b_jump = SRAM_readByte(CFG_OFF + 5) & 3;
    config.b_fire = SRAM_readByte(CFG_OFF + 6) & 3;
    config.b_dash = SRAM_readByte(CFG_OFF + 7) & 3;
    config.game_finished = SRAM_readByte(CFG_OFF + 8);
    if (config.b_jump > 2) config.b_jump = 2;
    if (config.b_fire > 2) config.b_fire = 1;
    if (config.b_dash > 2) config.b_dash = 0;
    sfx_set_enabled(config.sound != 0);   // music is gated at stage start
    music_set_volume(config.music_vol);
}

void config_save(void)
{
    SRAM_writeWord(CFG_OFF, CFG_MAGIC);
    SRAM_writeByte(CFG_OFF + 2, config.sound);
    SRAM_writeByte(CFG_OFF + 9, (u8)(CFG_VOL_SET | config.music_vol));
    SRAM_writeByte(CFG_OFF + 3, config.turbo);
    SRAM_writeByte(CFG_OFF + 4, config.autocharge);
    SRAM_writeByte(CFG_OFF + 5, config.b_jump);
    SRAM_writeByte(CFG_OFF + 6, config.b_fire);
    SRAM_writeByte(CFG_OFF + 7, config.b_dash);
    SRAM_writeByte(CFG_OFF + 8, config.game_finished);
}

bool config_is_sound_on(void) { return config.sound != 0; }

// Test builds run silent (owner's request, 2026-10-25): the XGM2 driver and the
// PCM effects cost Z80/68k time that muddies profile numbers, and an automated
// run should not need the host's audio path at all. The option itself stays in
// the config screen, so a human run can turn sound back on.
void config_force_silent(void)
{
    config.sound = 0;
    sfx_set_enabled(0);
    music_stop();
}

static u16 button_of(u8 idx)
{
    return (idx == 0) ? BUTTON_A : (idx == 1) ? BUTTON_B : BUTTON_C;
}

u16 config_jump_mask(void) { return button_of(config.b_jump); }
u16 config_fire_mask(void) { return button_of(config.b_fire); }
u16 config_dash_mask(void) { return button_of(config.b_dash); }

// Buttons that pick a menu item: A, plus the player's JUMP button (B stays
// BACK even when jump is mapped to it).
static u16 sel_mask(void)
{
    return BUTTON_A | (config_jump_mask() & ~BUTTON_B);
}

static void apply_sound(void)
{
    sfx_set_enabled(config.sound != 0);
    if (config.sound) {
        music_resume();
    } else {
        music_stop();
    }
}

// Solid tile for the options background. Tile 935 is the last user tile
// before the sprite pool (936): the stage tileset/parallax/anim blocks all
// end below it (checked by make check-vram), and transitions only borrow the
// block during a room slide, never while the menu is open.
// Below the sprite pool (src/stage.h): this screen is reachable from
// the in-game menu, so a fixed tile inside the pool would overwrite a
// live sprite's art.
#define CFG_BLACK_TILE SPR_CFG_BLACK_TILE
// Solid black fill (BUG-044). 0x11111111 sets bitplane 0 on all 8 pixels of
// the row = palette index 1; the old 0x11111111 set bitplane 0..3 on pixels 3
// and 7 only, which draws index 0xF - the lightest entry of whatever palette
// the screen inherited - i.e. two bright stripes per tile. This tile is loaded
// at SPR_CFG_BLACK_TILE, which is also stage_sky_tile(), so the stripes leaked
// into the stage sky and the room-transition band fills.
static const u32 cfg_black[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
};

static void clear_screen(void)
{
    // CPU upload, not DMA: this screen is entered with the display ON (in-game
    // menu), and an immediate DMA started outside vblank is the same class of
    // illegal transfer as a CPU-method CRAM write (debugging.md FIND-08) - the
    // tile ends up holding something other than cfg_black.
    VDP_loadTileData(cfg_black, CFG_BLACK_TILE, 1, CPU);
    // index 1 has to BE black: this screen inherits PAL0 from whoever opened it
    // (title logo line 1 is white, the stage line 1 is a stage colour).
    pal_color(1, 0x0000);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    // opaque background: the font's space tile is transparent and would show
    // the stage parallax through the options screen
    // priority TRUE on purpose: the options screen must win against whatever
    // BG_B still holds (the stage parallax / the title logo). With priority
    // FALSE this fill loses every pixel to a BG_B cell and the screen shows the
    // plane below instead of black (stripes, BUG-044 follow-up).
    // Split fill: LEFT half = tile 0 (known blank -> shows the black backdrop),
    // RIGHT half = our solid tile. If only the right half stripes, the tile's
    // content is wrong; if both do, the plane fill itself is not landing.
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE, 0),
                        0, 0, 20, 28);
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE,
                                       CFG_BLACK_TILE),
                        20, 0, 20, 28);
    tdump_vdp("config-fill");
    tdump_cram("config-fill");
    tdump_tile("config-fill", CFG_BLACK_TILE);
    tdump_cell("config-fill", VDP_getBGAAddress(), 0, 0);
    tdump_cell("config-fill", VDP_getBGAAddress(), 12, 12);
    tdump_cell("config-fill", VDP_getBGBAddress(), 12, 12);
}

static void wait_release(void)
{
    u16 pad;
    do {
        SYS_doVBlankProcess();
        pad = JOY_readJoypad(JOY_1);
    } while (pad & (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START));
}

static void draw_list(const char *title, const char *const *items, u8 n,
                      u8 sel)
{
    clear_screen();
    VDP_drawText(title, 2, 2);
    for (u8 i = 0; i < n; i++)
        VDP_drawText(items[i], 5, 6 + i * 2);
    VDP_drawText(">", 3, 6 + sel * 2);
    VDP_drawText("A:SELECT  B:BACK", 2, 26);
#ifdef MD_TILE_DUMP
    {
        static bool dumped;
        if (!dumped) {
            dumped = TRUE;
            SYS_doVBlankProcess();          // flush the queued text writes
            tdump_vdp("config-drawn");
            tdump_cram("config-drawn");
            tdump_tile("config-drawn", CFG_BLACK_TILE);
            tdump_tile("config-drawn", 0);
            tdump_cell("config-drawn", VDP_getBGAAddress(), 0, 6);
            tdump_cell("config-drawn", VDP_getBGAAddress(), 2, 2);
            tdump_cell("config-drawn", VDP_getBGBAddress(), 12, 12);
            tdump_sprites("config-drawn");
        }
    }
#endif
}

// Returns the picked row, or -1 on BACK.
static s8 menu_list(const char *title, const char *const *items, u8 n, u8 *sel)
{
    for (;;) {
        draw_list(title, items, n, *sel);
        u16 pad;
        do {
            SYS_doVBlankProcess();
            pad = JOY_readJoypad(JOY_1);
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | sel_mask() | BUTTON_B |
                          BUTTON_START)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_UP | BUTTON_DOWN | sel_mask() |
                                        BUTTON_B | BUTTON_START))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return -1;
        if (pad & (sel_mask() | BUTTON_START)) return (s8)*sel;
        if (pad & BUTTON_UP) *sel = (*sel == 0) ? n - 1 : *sel - 1;
        if (pad & BUTTON_DOWN) *sel = (*sel + 1) % n;
    }
}

static void audio_menu(void)
{
    // PC config has volume_music and volume_sfx. XGM2 exposes a master volume
    // for the music halves only (XGM2_setFMVolume / setPSGVolume); the PCM
    // effects have no runtime volume, so SFX stay on/off.
    u8 row = 0;
    for (;;) {
        char music[20] = "MUSIC VOL: 3";
        char sound[24] = "SOUND: ";
        music[11] = (char)('0' + config.music_vol);
        strcat(sound, config.sound ? "ON" : "OFF");
        const char *items[2] = { music, sound };
        draw_list("AUDIO", items, 2, row);
        VDP_drawText("A:CHANGE  B:BACK", 2, 26);
        u16 pad;
        do {
            SYS_doVBlankProcess();
            pad = JOY_readJoypad(JOY_1);
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | sel_mask() | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_UP | BUTTON_DOWN | sel_mask() |
                                        BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (pad & BUTTON_UP)   row = (row == 0) ? 1 : row - 1;
        if (pad & BUTTON_DOWN) row = (row + 1) % 2;
        if (pad & sel_mask()) {
            if (row == 0) {
                config.music_vol = (u8)((config.music_vol + 1) & 3);
                music_set_volume(config.music_vol);
            } else {
                config.sound = !config.sound;
                apply_sound();
            }
            config_save();
        }
    }
}

static void buttons_menu(void)
{
    static const char *const letters[3] = { "A", "B", "C" };
    u8 row = 0;
    for (;;) {
        char l0[20], l1[20], l2[20];
        strcpy(l0, "JUMP:  ");
        strcat(l0, letters[config.b_jump]);
        strcpy(l1, "FIRE:  ");
        strcat(l1, letters[config.b_fire]);
        strcpy(l2, "DASH:  ");
        strcat(l2, letters[config.b_dash]);
        const char *items[3] = { l0, l1, l2 };
        draw_list("CONFIG BUTTONS", items, 3, row);
        VDP_drawText("A:CHANGE  B:BACK", 2, 26);
        u16 pad;
        do {
            SYS_doVBlankProcess();
            pad = JOY_readJoypad(JOY_1);
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | sel_mask() | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_UP | BUTTON_DOWN | sel_mask() |
                                        BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (pad & BUTTON_UP) row = (row == 0) ? 2 : row - 1;
        if (pad & BUTTON_DOWN) row = (row + 1) % 3;
        if (pad & sel_mask()) {
            // Cycle A->B->C; if another action owns that button, swap so two
            // actions never share a button.
            u8 *slot = (row == 0) ? &config.b_jump
                       : (row == 1) ? &config.b_fire : &config.b_dash;
            u8 old = *slot;
            u8 next = (u8)((old + 1) % 3);
            u8 *other = (config.b_jump == next) ? &config.b_jump
                        : (config.b_fire == next) ? &config.b_fire
                        : (config.b_dash == next) ? &config.b_dash : NULL;
            if (other && other != slot) *other = old;
            *slot = next;
            config_save();
            sfx_play(SFX_CURSOR);
        }
    }
}

static void input_menu(void)
{
    u8 sel = 0;
    for (;;) {
        char turbo[24] = "TURBO MODE: ";
        strcat(turbo, config.turbo ? "ON" : "OFF");
        char autoch[24] = "AUTO-CHARGE: ";
        strcat(autoch, config.autocharge ? "ON" : "OFF");
        const char *items[3] = { turbo, autoch, "CONFIG BUTTONS" };
        s8 r = menu_list("INPUT", items, 3, &sel);
        if (r < 0) return;
        if (r == 0) { config.turbo = !config.turbo; config_save(); }
        else if (r == 1) { config.autocharge = !config.autocharge; config_save(); }
        else buttons_menu();
    }
}

static void music_test(void)
{
    for (;;) {
        clear_screen();
        VDP_drawText("MUSIC TEST", 2, 2);
        VDP_drawText("A: PLAY/STOP", 2, 8);
        VDP_drawText("B: BACK", 2, 10);
        u16 pad;
        do {
            SYS_doVBlankProcess();
            pad = JOY_readJoypad(JOY_1);
        } while (!(pad & (sel_mask() | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_UP | BUTTON_DOWN | sel_mask() |
                                        BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (config.sound) {
            music_play(MUSIC_STAGE0);
        }
    }
}

static void sfx_test(void)
{
    u8 sel = 0;
    for (;;) {
        clear_screen();
        VDP_drawText("SFX TEST", 2, 2);
        VDP_drawText(sfx_names[sel], 5, 7);
        VDP_drawText("A: PLAY  UP/DOWN: SELECT", 2, 24);
        VDP_drawText("B: BACK", 2, 26);
        u16 pad;
        do {
            SYS_doVBlankProcess();
            pad = JOY_readJoypad(JOY_1);
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | sel_mask() | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_UP | BUTTON_DOWN | sel_mask() |
                                        BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (pad & sel_mask()) sfx_play((SfxId)sel);
        if (pad & BUTTON_UP) sel = (sel == 0) ? SFX_COUNT - 1 : sel - 1;
        if (pad & BUTTON_DOWN) sel = (u8)((sel + 1) % SFX_COUNT);
    }
}

static void extras_menu(void)
{
    u8 sel = 0;
    const char *items[3] = { "MUSIC TEST", "SFX TEST", "CREDITS" };
    for (;;) {
        s8 r = menu_list("EXTRAS", items, 3, &sel);
        if (r < 0) return;
        if (r == 0) music_test();
        else if (r == 1) sfx_test();
        else ending_credits_run();
    }
}

// PC dialogs::show_leave_game_dialog(): yes/no, defaults to NO; B = NO.
static bool confirm_leave(void)
{
    const char *items[2] = { "NO", "YES" };
    u8 sel = 0;
    return menu_list("LEAVE THE STAGE?", items, 2, &sel) == 1;
}

bool config_run(bool in_game)
{
    // The caller opens this with B still held: wait for the release or the
    // first menu_list would see that same press and bounce straight back.
    wait_release();
    u8 sel = 0;
    for (;;) {
        const char *items[4] = { "AUDIO", "INPUT", "EXTRAS", "LEAVE STAGE" };
        u8 n = in_game ? 4 : 3;
        s8 r = menu_list("OPTIONS", items, n, &sel);
        if (r < 0) return FALSE;
        if (r == 0) audio_menu();
        else if (r == 1) input_menu();
        else if (r == 2) extras_menu();
        else if (r == 3) {
            if (!confirm_leave()) continue;
            config_save();
            return TRUE;
        }
    }
}

#ifdef MD_MINIMAL
// Plain menu app: only what the screen itself needs. Boot lands here and stays.
void config_minimal_app(void)
{
    VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    VDP_setHorizontalScroll(BG_B, 0);
    VDP_setVerticalScroll(BG_B, 0);
    VDP_setBackgroundColor(0);

    // Everything that writes CRAM/VRAM happens with the display OFF: a
    // transfer whose address port is taken by the vblank handler mid-way lands
    // at the wrong destination (proved for CRAM: the boot ramp slid 8 or 10
    // entries down). If the stripes are this class, this screen comes out clean.
    VDP_setEnable(FALSE);
    pal_load(PAL0, pal_ui.data);          // the ONLY palette load in this build
    VDP_setTextPlane(BG_A);
    VDP_setTextPalette(PAL0);
    VDP_clearPlane(BG_B, TRUE);

    VDP_loadTileData(cfg_black, CFG_BLACK_TILE, 1, CPU);
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, TRUE, FALSE, FALSE,
                                       CFG_BLACK_TILE),
                        0, 0, 40, 28);
    VDP_drawText("OPTIONS", 2, 2);
    VDP_drawText("> AUDIO", 5, 6);
    VDP_drawText("  INPUT", 5, 8);
    VDP_drawText("  EXTRAS", 5, 10);
    VDP_drawText("A:SELECT  B:BACK", 2, 26);
    VDP_setEnable(TRUE);

    SYS_doVBlankProcess();
    tdump_vdp("minimal");
    tdump_cram("minimal");
    tdump_tile("minimal", CFG_BLACK_TILE);
    tdump_tile("minimal", 0);
    tdump_cell("minimal", VDP_getBGAAddress(), 0, 0);
    tdump_cell("minimal", VDP_getBGAAddress(), 2, 2);
    tdump_cell("minimal", VDP_getBGBAddress(), 12, 12);
    tdump_sprites("minimal");

    for (;;) SYS_doVBlankProcess();
}
#endif
