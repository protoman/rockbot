#ifndef CONFIG_H
#define CONFIG_H

#include <genesis.h>

// Simplified MD options (PC game_menu::show_main_config, reduced per user):
// AUDIO, INPUT, EXTRAS and LEAVE STAGE. No platform/language/performance/
// quit-game entries; the PC analog/digital + joystick input rows are gone.
//
// Persisted in SRAM right after the 5 save slots (bytes 48..56).
#ifdef MD_MINIMAL
// Bisection build: a hand-drawn options screen with nothing else initialised
// (no stage, no sprites, no music, no SRAM). Never returns.
void config_minimal_app(void);
#endif

typedef struct {
    u8 sound;       // 1 = SFX+music on
    u8 music_vol;   // PC volume_music, 4 steps (0..3)
    u8 turbo;       // hold fire = repeat shot (PC turbo_mode)
    u8 autocharge;  // default weapon auto-fires (PC auto_charge_mode)
    u8 b_jump;      // 0=A 1=B 2=C
    u8 b_fire;
    u8 b_dash;
    u8 game_finished; // PC game_config.game_finished: unlocks players 3/4
} Config;

extern Config config;

void config_init(void);            // load (defaults on first boot)
void config_save(void);
bool config_is_sound_on(void);
// Test builds: sound off regardless of what SRAM says.
void config_force_silent(void);

// Button masks for the player input (config.h: 0=A, 1=B, 2=C).
u16 config_jump_mask(void);
u16 config_fire_mask(void);
u16 config_dash_mask(void);

// Options screen. Returns TRUE when the player picked LEAVE STAGE.
bool config_run(bool in_game);

#endif
