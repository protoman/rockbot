#!/usr/bin/env python3
"""Convert the PC MOD/S3M music to VGM for the XGM2 driver.

The mod2vgm tool in tools/mod2vgm/build turns the Amiga modules into standard
VGM; rescomp's XGM2 processor then packs those VGMs into the ROM as XGM2 blobs
(see res/resources.res).

The PC data has one MP3 (classic_style_castle_intro.mp3). The port uses
rockbot_stage_start.mod for that slot instead, as requested.

Every source module is converted, including the alternate/unused files, so a
later UI can expose them without changing the build.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
MUSIC_SRC = os.path.join(ROOT, "data/games/Rockbot1/music")
RES = os.path.join(ROOT, "res/music")
TOOL = os.path.join(HERE, "mod2vgm/build/mod2vgm")

# (output resource name, PC source file)
TRACKS = [
    ("mus_opening",         "rockbot_opening.mod"),
    ("mus_intro_scene",     "intro_scene.mod"),
    ("mus_selector",        "rockbot_selector.mod"),
    ("mus_selector_alt",    "rockbot_selector_0021.mod"),
    ("mus_boss_intro",      "classic_style_boss_intro.s3m"),
    # classic_style_castle_intro.mp3 is replaced by this MOD.
    ("mus_castle_intro",    "rockbot_stage_start.mod"),
    ("mus_castle_intro_alt", "rockbot_skull_castle_intro.mod"),
    ("mus_boss",            "rockbot_boss.mod"),
    ("mus_final_boss",      "final_boss.mod"),
    ("mus_got_weapon",      "rockbot_weapon_aquired.mod"),
    ("mus_game_over",       "rockbot_gameover.mod"),
    ("mus_stage_complete",  "rockbot_stage_complete_00.mod"),
    ("mus_enemy_parade",    "rockbot_enemy_parade.mod"),
    ("mus_end_animation",   "rockbot_end_animation.mod"),
    ("mus_end_credits",     "rockbot_endcredits.mod"),
    ("mus_transporters",    "rockbot_transporters.mod"),
    ("mus_stage0",          "rockbot_chaos_city.mod"),
    ("mus_stage1",          "rockbot_jungle.mod"),
    ("mus_stage2",          "rockbot_daisy_bot.mod"),
    ("mus_stage3",          "rockbot_seahorsebot.mod"),
    ("mus_stage4",          "rockbot_mummybot.mod"),
    ("mus_stage5",          "rockbot_trickycastle.mod"),
    ("mus_stage6",          "rockbot_minerbot_.mod"),
    ("mus_stage7",          "rockbot_spikebot.mod"),
    ("mus_stage8",          "rockbot_technobot.mod"),
    ("mus_stage9",          "rockbot_skull_castle_0.mod"),
    ("mus_stage10",         "rockbot_skull_castle_I.mod"),
    ("mus_stage11",         "rockbot_skull_castle_II.mod"),
    ("mus_stage12",         "rockbot_skull_castle_III.mod"),
    ("mus_stage13",         "rockbot_skull_castle_IV.mod"),
]


def main():
    if not os.path.isfile(TOOL):
        sys.exit("convert_music: missing converter: %s" % TOOL)
    os.makedirs(RES, exist_ok=True)
    for name, src_name in TRACKS:
        src = os.path.join(MUSIC_SRC, src_name)
        dst = os.path.join(RES, name + ".vgm")
        if not os.path.isfile(src):
            sys.exit("convert_music: missing source: %s" % src)
        print("music: %s -> %s" % (src_name, os.path.basename(dst)))
        subprocess.run([TOOL, src, dst], check=True)
    print("music: %d tracks converted" % len(TRACKS))


if __name__ == "__main__":
    main()
