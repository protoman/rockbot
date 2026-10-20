#include "config.h"

#include <genesis.h>

#include "resources.h"   // stage_music
#include "sfx.h"
#include "ending.h"

// SRAM: 5 save slots occupy bytes 8..47; options live at byte 48.
#define CFG_OFF   48
#define CFG_MAGIC 0x4F50       // 'OP'

Config config;

static const char *const sfx_names[SFX_COUNT] = {
    "SHOT", "CHARGED SHOT", "JUMP", "PLAYER HIT", "PLAYER DEATH",
    "CHARGING 1", "CHARGING 2", "ENEMY HIT", "ENEMY KILLED", "EXPLOSION",
    "ENERGY PILL", "ITEM", "TELEPORT", "WATER ENTER", "WATER LEAVE",
    "CURSOR",
};

void config_init(void)
{
    if (SRAM_readWord(CFG_OFF) != CFG_MAGIC) {
        config.sound = 1;
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
}

void config_save(void)
{
    SRAM_writeWord(CFG_OFF, CFG_MAGIC);
    SRAM_writeByte(CFG_OFF + 2, config.sound);
    SRAM_writeByte(CFG_OFF + 3, config.turbo);
    SRAM_writeByte(CFG_OFF + 4, config.autocharge);
    SRAM_writeByte(CFG_OFF + 5, config.b_jump);
    SRAM_writeByte(CFG_OFF + 6, config.b_fire);
    SRAM_writeByte(CFG_OFF + 7, config.b_dash);
    SRAM_writeByte(CFG_OFF + 8, config.game_finished);
}

bool config_is_sound_on(void) { return config.sound != 0; }

static u16 button_of(u8 idx)
{
    return (idx == 0) ? BUTTON_A : (idx == 1) ? BUTTON_B : BUTTON_C;
}

u16 config_jump_mask(void) { return button_of(config.b_jump); }
u16 config_fire_mask(void) { return button_of(config.b_fire); }
u16 config_dash_mask(void) { return button_of(config.b_dash); }

static void apply_sound(void)
{
    sfx_set_enabled(config.sound != 0);
    if (config.sound) {
        XGM2_play(stage_music);
    } else {
        XGM2_stop();
    }
}

// Solid tile for the options background. Tile 935 is the last user tile
// before the sprite pool (936): the stage tileset/parallax/anim blocks all
// end below it (checked by make check-vram), and transitions only borrow the
// block during a room slide, never while the menu is open.
#define CFG_BLACK_TILE 935
static const u32 cfg_black[8] = {
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
    0x11111111, 0x11111111, 0x11111111, 0x11111111,
};

static void clear_screen(void)
{
    VDP_loadTileData(cfg_black, CFG_BLACK_TILE, 1, DMA);
    VDP_setHorizontalScroll(BG_A, 0);
    VDP_setVerticalScroll(BG_A, 0);
    // opaque background: the font's space tile is transparent and would show
    // the stage parallax through the options screen
    VDP_fillTileMapRect(BG_A,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE,
                                       CFG_BLACK_TILE),
                        0, 0, 40, 28);
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
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | BUTTON_A | BUTTON_B |
                          BUTTON_START)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_A | BUTTON_B | BUTTON_START))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return -1;
        if (pad & (BUTTON_A | BUTTON_START)) return (s8)*sel;
        if (pad & BUTTON_UP) *sel = (*sel == 0) ? n - 1 : *sel - 1;
        if (pad & BUTTON_DOWN) *sel = (*sel + 1) % n;
    }
}

static void audio_menu(void)
{
    u8 sel = 0;
    for (;;) {
        char sound[24] = "SOUND: ";
        strcat(sound, config.sound ? "ON" : "OFF");
        const char *items[1] = { sound };
        s8 r = menu_list("AUDIO", items, 1, &sel);
        if (r < 0) return;
        config.sound = !config.sound;
        apply_sound();
        config_save();
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
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | BUTTON_A | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_A | BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (pad & BUTTON_UP) row = (row == 0) ? 2 : row - 1;
        if (pad & BUTTON_DOWN) row = (row + 1) % 3;
        if (pad & BUTTON_A) {
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
        } while (!(pad & (BUTTON_A | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_A | BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (config.sound) {
            XGM2_stop();
            XGM2_play(stage_music);
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
        } while (!(pad & (BUTTON_UP | BUTTON_DOWN | BUTTON_A | BUTTON_B)));
        while (JOY_readJoypad(JOY_1) & (BUTTON_A | BUTTON_B))
            SYS_doVBlankProcess();
        if (pad & BUTTON_B) return;
        if (pad & BUTTON_A) sfx_play((SfxId)sel);
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
            config_save();
            return TRUE;
        }
    }
}
