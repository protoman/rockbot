#ifndef MUSIC_H
#define MUSIC_H

#include <genesis.h>

// Music tracks converted from the PC MOD/S3M sources by tools/convert_music.py
// and packed as XGM2 resources (res/resources.res).
typedef enum {
    MUSIC_NONE = 0,
    MUSIC_OPENING,
    MUSIC_INTRO_SCENE,
    MUSIC_SELECTOR,
    MUSIC_SELECTOR_ALT,
    MUSIC_BOSS_INTRO,
    MUSIC_CASTLE_INTRO,
    MUSIC_CASTLE_INTRO_ALT,
    MUSIC_BOSS,
    MUSIC_FINAL_BOSS,
    MUSIC_GOT_WEAPON,
    MUSIC_GAME_OVER,
    MUSIC_STAGE_COMPLETE,
    MUSIC_ENEMY_PARADE,
    MUSIC_END_ANIMATION,
    MUSIC_END_CREDITS,
    MUSIC_TRANSPORTERS,
    MUSIC_STAGE0,
    MUSIC_STAGE1,
    MUSIC_STAGE2,
    MUSIC_STAGE3,
    MUSIC_STAGE4,
    MUSIC_STAGE5,
    MUSIC_STAGE6,
    MUSIC_STAGE7,
    MUSIC_STAGE8,
    MUSIC_STAGE9,
    MUSIC_STAGE10,
    MUSIC_STAGE11,
    MUSIC_STAGE12,
    MUSIC_STAGE13,
    MUSIC_COUNT
} MusicId;

// Start a track (loops until another track is started or music_stop()).
// No-op when sound is disabled in CONFIG or the same track is already active.
void music_play(MusicId id);

// Start the stage's PC background music (stage 0..13).
void music_play_stage(u8 stage);

// Stop the current music; the track is remembered for music_resume().
void music_stop(void);

// PC volume_music: 0..3 steps (0 = silent ... 3 = full).
void music_set_volume(u8 level);
u8 music_get_volume(void);

// Re-start the remembered track (used when sound is re-enabled in CONFIG).
void music_resume(void);

#endif
