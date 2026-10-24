#include "sprite_guard.h"

// Cap the diagnostics: one bad index usually repeats every frame, and the log
// only needs the first few offenders to identify the call site's art.
#define SPR_BAD_LOGS_MAX 12

static u16 bad_logs;

static void report(const char *what, s16 index, u16 count,
                   const SpriteDefinition *def)
{
    if (bad_logs >= SPR_BAD_LOGS_MAX) return;
    if (bad_logs == 0)
        kprintf("sprite guard: %s out of range (index %d, %u valid, def %p)\n",
                what, index, count, def);
    else
        kprintf("sprite guard: %s index %d >= %u\n", what, index, count);
    bad_logs++;
    if (bad_logs == SPR_BAD_LOGS_MAX)
        kprintf("sprite guard: further reports suppressed\n");
}

void spr_set_anim(Sprite *spr, s16 anim)
{
    if (!spr) return;
    const SpriteDefinition *def = spr->definition;
    const u16 n = def ? def->numAnimation : 0;
    if (!def || anim < 0 || (u16)anim >= n) {
        report("animation", anim, n, def);
        return;
    }
    SPR_setAnim(spr, anim);
}

void spr_set_frame(Sprite *spr, s16 frame)
{
    if (!spr) return;
    const Animation *a = spr->animation;
    const u16 n = a ? a->numFrame : 0;
    if (!a || frame < 0 || (u16)frame >= n) {
        report("frame", frame, n, spr->definition);
        return;
    }
    SPR_setFrame(spr, frame);
}
