#!/usr/bin/env python3
"""Offline check: the boss-intro jingle must be played only together with the
boss-intro presentation.

BUG it reproduces (2026-10-30): replaying an already-beaten stage skipped the
presentation but the 6.4 s boss-intro jingle was still heard. The port decided
"beaten" in the *caller* (main.c runStage) and played the music inside
`boss_intro_run`, so the jingle was reachable without the screen - and
`music_stop()` returned early while the port believed nothing was playing,
which can leave a stale track running (a stopped-but-still-audible jingle is
exactly what a skipped intro would sound like).

The PC decides it at the top of the intro itself:
`scenesLib::classic_style_boss_intro` starts with
`if (game_save.stages[stage_n] != 0) return;` (sceneslib.cpp:682), before any
screen or music. These rules check the port keeps that shape:

  1. `boss_intro_run` gates on the beaten mask *itself*, before its first
     screen or music call;
  2. MUSIC_BOSS_INTRO is played from bossintro.c only - never from main.c;
  3. `music_stop()` always stops the XGM2 driver (no early return on the
     port's own `playing` flag, which can disagree with the driver).

Usage: tools/check_boss_intro_gate.py     exit 1 on a violation.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(os.path.dirname(HERE), "src")


def read(name):
    with open(os.path.join(SRC, name), encoding="utf-8") as fp:
        return fp.read()


def function_body(text, name):
    """Body of `name(...) { ... }` (first definition, brace matched)."""
    m = re.search(r"^[A-Za-z_][\w \*]*\b%s\s*\([^;]*?\)\s*\{" % name,
                  text, re.M)
    if not m:
        return None
    depth, i = 0, m.end() - 1
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[m.end():i]
        i += 1
    return None


def main():
    errors = []
    boss = read("bossintro.c")
    music = read("music.c")
    main_c = read("main.c")

    body = function_body(boss, "boss_intro_run")
    if body is None:
        errors.append("bossintro.c: no boss_intro_run definition found")
    else:
        gate = re.search(r"beaten[\w_]*\s*&\s*\(1u?\s*<<\s*stage\)", body)
        first_call = min([m.start() for m in re.finditer(
            r"\b(music_play|castle_screen|VDP_loadTileData|"
            r"SPR_addSprite|PAL_setPalette)\s*\(", body)] or [len(body)])
        if not gate:
            errors.append("bossintro.c: boss_intro_run does not check the "
                          "beaten-stage mask itself")
        elif gate.start() > first_call:
            errors.append("bossintro.c: boss_intro_run draws/plays (offset %d) "
                          "before the beaten check (offset %d)"
                          % (first_call, gate.start()))
        if "return FALSE" not in body[:first_call + 1]:
            errors.append("bossintro.c: boss_intro_run must return FALSE when "
                          "the stage is already beaten")

    if "MUSIC_BOSS_INTRO" in main_c:
        errors.append("main.c: the boss-intro jingle must only be started by "
                      "bossintro.c (presentation and music gated together)")

    stop = function_body(music, "music_stop")
    if stop is None:
        errors.append("music.c: no music_stop definition found")
    elif re.search(r"if\s*\(\s*!\s*playing\s*\)\s*return", stop):
        errors.append("music.c: music_stop early-returns on the port's own "
                      "'playing' flag; a desynced flag leaves the previous "
                      "track (e.g. a skipped intro's jingle) audible")

    for e in errors:
        print("FAIL  " + e)
    print("%d boss-intro gate violation(s)" % len(errors))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
