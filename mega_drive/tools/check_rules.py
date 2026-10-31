#!/usr/bin/env python3
"""Source invariants for interaction bugs that cannot be driven offline.

Some reports are pure gameplay interaction (a door re-opening, knockback
pushing the player into an object, a slide through a scroll lock). There is no
headless harness for the 68k here, so the reproduction is a **rule check**: each
entry states the PC behaviour it encodes, the file it lives in and the code
shape that must exist. It fails on the tree *before* the fix (see
`--rev`), which is how the fix was verified without the emulator:

    tools/check_rules.py --rev HEAD   # old tree: these rules fail
    tools/check_rules.py              # fixed tree: all pass

Numbers in brackets are the reports from 2026-10-24.
"""
import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (id, description, [(file, regex)...], [forbidden regexes on that file])
RULES = [
    ("door-once",
     "a boss door opens once; touching it again must not scroll the room",
     [("src/object.c", r"if \(o->started\) continue;"),
      ("src/object.c", r"o->started = TRUE;")],
     []),
    ("door-scroll-sync",
     "the door's slice sprites follow the camera during a screen move",
     [("src/object.c", r"door_sync\(o, on_screen\);")],
     []),
    ("pushback-collision",
     "knockback stops at solid map objects (player cannot land inside a door)",
     [("src/player.c", r"objects_block_x\(ix, iy \+ box_t, iy \+ box_b, pdx, &px\)")],
     []),
    ("slide-scroll-lock",
     "a slide triggers a horizontal scroll lock, and the probe covers the sweep",
     [("src/player.c", r"if \(!dir && player\.sliding\) dir ="),
      ("src/player.c", r"for \(s16 off = -reach; off <= 2; off\+\+\)")],
     [("src/player.c", r"if \(player\.on_ladder \|\| player\.sliding \|\| player\.hit_push > 0\) return;")]),
    ("enemy-object-collision",
     "enemies stop at solid map objects instead of walking through them",
     [("src/enemy.c", r"objects_block_box\(lead, e->y \+ d->box_t")],
     []),
    ("transition-plane-address",
     "VDP_setTileMapData takes the plane ADDRESS (VDP_BG_B) and a byte step of 2; "
     "passing the BG_B enum wrote the parallax rows over VRAM 0.. (the outgoing "
     "tileset) and left the auto-increment at 1 (the transition artifacts)",
     [("src/transition.c", r"VDP_setTileMapData\(VDP_BG_B, bgrow, \(u16\)\(r \* 64\), 64, 2,")],
     [("src/transition.c", r"VDP_setTileMapData\(\s*(BG_[AB]|WINDOW)\b"),
      ("src/transition.c", r"VDP_setTileMapData\([^;]*,\s*64,\s*1,")]),
    ("link-ladder-arrival-column",
     "the ladder scan on a room change starts at the real arrival column "
     "(dst_x + band offset), not only at dst_x: wide links land far from dst_x",
     [("src/main.c", r"s16 arr_tx = \(s16\)\(\(to_x \+ stage_link_band_off\) / tile\);"),
      ("src/main.c", r"const s16 bases\[2\] = \{ arr_tx, dst_tx \};")],
     []),
    ("transition-player-sprite-offset",
     "the slide draws the player with the same sprite offset as player_update "
     "(flip + ladder art), otherwise he is ~5px off the ladder for the whole slide",
     [("src/transition.c", r"SPR_setPosition\(player\.spr, \(s16\)\(player_sx \+ player_sprite_dx\(\)\), py\);"),
      ("src/player.c", r"s16 player_sprite_dx\(void\)")],
     [("src/transition.c", r"SPR_setPosition\(player\.spr, player_sx, py\);")]),
    ("transition-outgoing-shift",
     "the slide moves the OUTGOING screen by the residual the destination camera "
     "cannot absorb, so both ladders share one screen column (3px off at map edges)",
     [("src/transition.c", r"const s16 ocam = clamp_cam_x\(clamp_cam_x\(camera\.x\) - old_shift\);"),
      ("src/main.c", r"- cam_x\) - keep_sx\)\);")],
     [("src/transition.c", r"const s16 ocam = clamp_cam_x\(camera\.x\);")]),
    ("menu-restores-visible-sprites",
     "closing the menu restores the sprites that were visible: off-screen objects "
     "(spr_vis cache) and dead enemies must not be shown (frozen items on screen)",
     [("src/object.c", r"const bool show = visible && objs\[i\]\.spr_vis != 0;"),
      ("src/enemy.c", r"const bool show = visible && e->alive &&")],
     [("src/object.c", r"SPR_setVisibility\(objs\[i\]\.spr, visible \? VISIBLE : HIDDEN\);"),
      ("src/enemy.c", r"SPR_setVisibility\(enemies\[i\]\.spr, visible \? VISIBLE : HIDDEN\);")]),
    ("enemy-shot-at-attack-frame",
     "an enemy's shot spawns when its attack animation reaches the data's "
     "attack_frame (PC ai.cpp:1185), not at a flat 30% of the animation "
     "(HELMUT shot ~440ms early)",
     [("src/enemy.c", r"static u16 ai_shot_frame\(const EnemyDef \*d, u16 dur\)"),
      ("src/enemy.c", r"e->act_t == 1 \+ ai_shot_frame\(d, dur\)"),
      ("src/stage_data.h", r"u16 attack_shot_ms;"),
      ("tools/convert_enemies.py", r"\"attack_frame\": b\[6034\]")],
     [("src/enemy.c", r"e->act_t == 1 \+ \(dur \* 3\) / 10")]),
    ("slide-trail",
     "a sliding/dashing player leaves the PC's after-image trail (character.cpp:899, "
     "12 stored positions): ghost sprites, flickered, hidden when the slide ends",
     [("src/player.c", r"static void trail_update\(s16 ix, s16 iy\)"),
      ("src/player.c", r"\n    trail_update\(ix, iy\);"),
      ("src/player.c", r"if \(!visible\) trail_hide\(\);"),
      ("src/player.c", r"ghost_spr\[k\] = SPR_addSprite\(player_defs\[player_n\]")],
     []),
    ("wall-bomb-only",
     "only an exploding bomb breaks a destructible wall (type 22)",
     [("src/object.c", r"if \(ty == 22 && !bomb\) continue;"),
      ("src/shot.c", r"objects_try_hit\(s->x - 20, s->y - 20, s->x \+ 20, s->y \+ 20,\s*\n?\s*s->dmg, TRUE\);")],
     []),
    ("air-walk-arrival",
     "AI_AIR_WALK finishes when the enemy reaches the target X (PC), so the APE "
     "BOT leaves the ceiling instead of parking there",
     [("src/enemy.c", r"return step_axis\(e, &e->x, e->dest_x, TRUE, TRUE\);")],
     [("src/enemy.c", r"case AI_AIR_WALK:[\s\S]{0,400}return on_ground\(e, d\);")]),
    ("door-locks-forever",
     "a boss door's columns stay camera-locked after it opens (PC "
     "wall_scroll_lock is built at map load)",
     [("src/object.c", r"if \(o->def->type != 30\) continue;")],
     [("src/object.c", r"if \(o->def->type != 30 \|\| o->started\) continue;")]),
    ("sky-blank-tile",
     "cells the parallax art does not cover use a real all-zero tile, not the "
     "font tile (which drew glyph pixels - white stripes over the bg)",
     [("src/stage.c", r"VDP_loadTileData\(zero_tile, SPR_CFG_BLACK_TILE"),
      ("src/transition.c", r"stage_sky_tile\(\)")],
     [("src/stage.c", r"TILE_FONT_INDEX\);\s*\n\s*for \(u16 y = 0; y < 32")]),
    ("spawn-cache-init",
     "every object spawn path fills the hot-loop caches (flags/grounded), or "
     "dropped items never fall",
     [("src/object.c", r"obj_init_cached\(o, d\)"),
      ("src/object.c", r"o->grounded = FALSE;")],
     []),
    ("bg-auto-scroll",
     "PC E_BG_SCROLL_MODE: a background with an auto mode moves by itself "
     "(LEFT used by RockDroid2 stage 3) instead of following the camera",
     [("src/stage.c", r"bg_auto_hscroll\(stage_current_map\)"),
      ("src/stage.c", r"stage_bg_auto_hscroll\(u8 map\)")],
     []),
    ("scene-op-guard",
     "a scene op the port cannot run is reported, never skipped silently",
     [("src/scene.c", r"scene %d: op %d not implemented")],
     []),
    ("unlock-hint",
     "players 3/4 are limited until the game is finished, and the screen says "
     "so (PC select_player + game_config.game_finished)",
     [("src/playerselect.c", r"FINISH THE GAME TO UNLOCK")],
     []),
    ("menu-armor-icons",
     "the in-game menu draws the PC armor-piece art (draw.cpp:1021)",
     [("src/menu.c", r"VDP_loadTileData\(menu_armor_tiles\[p\]\[mask\]")],
     []),
    ("music-volume",
     "PC volume_music is applied to XGM2 (FM + PSG) and survives a track change",
     [("src/music.c", r"XGM2_setFMVolume\(v\);"),
      ("src/music.c", r"music_set_volume\(music_level\);")],
     []),
    ("music-volume-migration",
     "a config saved before the music-volume option (byte +9 never written, 0) "
     "must load as the default level, not as silent (2026-10-31: ending test "
     "had no music at all; owner never touched the option)",
     [("src/config.c", r"CFG_VOL_SET"),
      ("src/config.c", r"SRAM_writeByte\(CFG_OFF \+ 9, \(u8\)\(CFG_VOL_SET")],
     []),
    ("menu-armor-line",
     "the in-game menu shows the installed armor pieces (PC "
     "draw::weapon_menu_show_player)",
     [("src/menu.c", r"draw_armor_line\(\);")],
     []),
    ("menu-tank-use",
     "the in-game menu can use a reserve tank (PC class_config.cpp:218 row 6 "
     "+ BTN_ATTACK); player_use_tank() used to have no caller",
     [("src/menu.c", r"player_use_tank\(menu_entry == MENU_ENTRY_ETANK\)")],
     []),
    ("dialog-keeps-parallax",
     "the dialog panel borrows PAL0, never PAL3 (the stage parallax), so the "
     "background behind the box keeps its colours",
     [("src/dialogs.c", r"#define DLG_CRAM 0"),
      ("src/dialogs.c", r"PAL_setPalette\(PAL0, pal, DMA_QUEUE\);")],
     [("src/dialogs.c", r"PAL_setPalette\(PAL3"),
      ("src/dialogs.c", r"#define PAL3_CRAM 48")]),
    ("weapon-menu-linear",
     "the weapon menu walks the linear weapon list with wrap (PC "
     "class_config.cpp:419), so every owned weapon is reachable",
     [("src/menu.c", r"for \(s16 i = \(s16\)cur \+ dir; i < n; i\+\+\)")],
     [("src/menu.c", r"const u8 row = \(u8\)\(cur / 4\);")]),
    ("ladder-semi-at-top",
     "the half-body ladder pose is the TOP-of-ladder state (feet on a rung, "
     "body middle past it - PC character.cpp:396/426), not a grab animation",
     [("src/player.c", r"if \(!mid_stair && feet_stair\) stair_frame = 1;")],
     [("src/player.c", r"ladder_semi\s*=\s*12;")]),
    ("arrival-band-nudge",
     "a stair arrival on the top/bottom row steps out of the return link's band "
     "so the latch can clear (BUG-005 regression: the call was lost)",
     [("src/main.c", r"stage_link_band_hit\(px \+ PLAYER_BOX_L,")],
     []),
    ("transition-row-split",
     "each transition scanline takes its source row (both planes) from the same "
     "tilemap index, so B2T does not apply the outgoing offset to the incoming "
     "screen",
     [("src/transition.c", r"const bool is_new = new_on_top \? \(prow < TRANS_ROWS\)"),
      ("src/transition.c", r"trans_hs_b\[y\] = is_new \? bg_new : bg_old;")],
     [("src/transition.c", r"\(y < \(u16\)split\) \? top_hs : bot_hs")]),
    ("spawn-own-span",
     "a spawned NPC gets its own patrol span, not the spawner's (ANGRY BUNNY)",
     [("src/enemy.c", r"const s16 half = cd->range \? \(s16\)cd->range : 48;")],
     [("src/enemy.c", r"return enemy_add\(i, at_self")]),
    ("teleport-options",
     "AI_TELEPORT honours its option and never moves Y (PC ia_action_teleport)",
     [("src/enemy.c", r"case AI_TELEPORT_LEFT:"),
      ("src/enemy.c", r"case AI_TELEPORT_TO_PLAYER:")],
     [("src/enemy.c", r"e->y = player\.y - 16;")]),
    ("hud-min-bar",
     "a live player/boss always shows at least one gauge segment",
     [("src/hud.c", r"if \(frame > 0 && frame < HUD_BAR_MIN_FRAME\) frame = HUD_BAR_MIN_FRAME;")],
     []),
]


def read(path, rev):
    if rev:
        try:
            out = subprocess.run(["git", "show", f"{rev}:{path}"], cwd=ROOT,
                                 capture_output=True, text=True, check=True)
            return out.stdout
        except subprocess.CalledProcessError:
            return ""
    try:
        return open(os.path.join(ROOT, path)).read()
    except OSError:
        return ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rev", help="check a git revision (reproduces the bugs)")
    args = ap.parse_args()
    cache = {}

    def text(path):
        if path not in cache:
            cache[path] = read(path, args.rev)
        return cache[path]

    failed = 0
    for rid, desc, needs, forbids in RULES:
        bad = [f"{f}: missing /{rx}/" for f, rx in needs
               if not re.search(rx, text(f))]
        bad += [f"{f}: still has /{rx}/" for f, rx in forbids
                if re.search(rx, text(f))]
        if bad:
            failed += 1
            print(f"FAIL {rid}: {desc}")
            for line in bad:
                print(f"     {line}")
        else:
            print(f"ok   {rid}")

    where = f" (rev {args.rev})" if args.rev else ""
    if failed:
        print(f"check_rules: {failed}/{len(RULES)} rule(s) violated{where}")
        return 1
    print(f"check_rules: OK ({len(RULES)} rules){where}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
