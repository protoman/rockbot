#ifndef SFX_H
#define SFX_H

#include <genesis.h>

// Sound effects: PC WAVs converted by tools/convert_sfx.py into res/sfx.res
// (rescomp WAV "XGM2" = 8 bit PCM at ~13.3 kHz) and played on the XGM2
// driver's PCM channels. Channel 1 is normally taken by the music PCM, so
// the effects use channels 2..4 (see xgm2.h).
typedef enum {
    SFX_SHOT = 0,       // player buster
    SFX_CHARGE,         // charged shot release
    SFX_JUMP,           // player jump
    SFX_PLAYER_HIT,     // player damaged
    SFX_PLAYER_DEATH,   // player death
    SFX_NPC_HIT,        // enemy damaged
    SFX_NPC_KILLED,     // enemy destroyed
    SFX_EXPLOSION,      // boss death
    SFX_PILL,           // small energy pill
    SFX_ITEM,           // 1UP
    SFX_TELEPORT,       // boss teleporter
    SFX_CURSOR,         // menu open/cursor
    SFX_COUNT
} SfxId;

// Fire-and-forget; ignored if a same-or-higher priority effect is playing on
// that channel, and a no-op for effects without a sample.
void sfx_play(SfxId id);

#endif
