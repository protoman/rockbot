#ifndef SPRITE_GUARD_H
#define SPRITE_GUARD_H

#include <genesis.h>

// Guarded sprite animation / frame changes.
//
// SGDK validates `SPR_setAnim` / `SPR_setFrame` arguments only when
// `LIB_LOG_LEVEL >= LOG_LEVEL_ERROR`, and a release build sets that to
// DISABLE (config.h: `DEBUG == 0` -> LIB_LOG_LEVEL DISABLE). An out-of-range
// index is then accepted, `SPR_update` reads `animation->frames[i]` past the
// end and dereferences whatever is there: BUG-031 (HUD ball frame) and
// BUG-043 (slide in the APE stage) were exactly that - an ADDRESS ERROR with
// an odd address inside `SPR_update`.
//
// These wrappers clamp/reject instead and print the offending index over
// KDEBUG (a few times, so a per-frame offender cannot flood the log).
void spr_set_anim(Sprite *spr, s16 anim);
void spr_set_frame(Sprite *spr, s16 frame);

#endif
