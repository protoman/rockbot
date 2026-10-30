#!/usr/bin/env python3
"""Offline check: the "got armor" dialog text and the port's armor tables must
match the ability the data actually installs.

BUG it reproduces (fixed): ROCKBOT gets the legs piece with ability 0 (double
jump) but the page read "DASH IN MIDDLE-AIR." - dialogs.c keyed the labels off
a different enum order than src/player.c uses. The body page was wrong for all
four values too.

Source of truth is the PC code + data, not a table copied here:
  ../defines.h             enum e_ARMOR_ABILITIES_LEGS / _BODY order
  ../scenes/dialogs.cpp    enum value -> message index (msg1..msg4), and the
                           ARMS page = "SHOOT <projectile name>."
  ../file/fio_strings.cpp  English text of that message
  data/games/RockDroid1/game_armorPieces_v301.dat   ability per piece/player
  data/games/RockDroid1/game_projectile_list.dat     projectile id -> name
and the port's own text/tables in src/dialogs.c and src/player.c are compared.

Covers all 4 players x 3 armor pieces.

Usage: tools/check_armor_dialog.py      exit 1 on a mismatch.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MD = os.path.dirname(HERE)
PC = os.path.dirname(MD)
DATA = os.path.join(MD, "data/games/RockDroid1")
PORT_DIALOGS = os.path.join(MD, "src/dialogs.c")
PORT_PLAYER = os.path.join(MD, "src/player.c")
PLAYERS = ("ROCKBOT", "BETABOT", "CANDYBOT", "KITTYBOT")
PROJ_REC = 104                  # sizeof(file_projectilev3) in game_projectile_list.dat


def read(path):
    with open(path, encoding="utf-8", errors="replace") as fp:
        return fp.read()


def enum_order(text, name):
    body = re.search(r"enum %s\s*\{(.*?)\}" % name, text, re.S).group(1)
    return [t for t in re.findall(r"\w+", body) if t.startswith("ARMOR_ABILITY")]


def pc_msg_index(pc_dialogs):
    """enum name -> fio_strings message suffix ('legs_msg1', ...)."""
    out = {}
    for name, kind, n in re.findall(
            r"type == (ARMOR_ABILITY_\w+)\)\s*\{\s*ability_str = [^;]*?"
            r"gotarmor_type_(legs|body)_msg(\d)", pc_dialogs, re.S):
        out[name] = "%s_msg%s" % (kind, n)
    return out


def english_texts(pc_strings):
    # fio_strings.cpp holds PT, ES and the English default block; the English
    # block comes last, so the last occurrence of each key is the EN text.
    keys = {}
    for kind, n, txt in re.findall(
            r'lines\[strings_ingame_gotarmor_type_(legs|body)_msg(\d)\]'
            r'[^;]*?"%s", "([^"]*)"', pc_strings):
        keys["%s_msg%s" % (kind, n)] = txt
    return keys


def pc_armor_data():
    """piece name -> [ability per player], from the .dat (stride 64 B)."""
    raw = open(os.path.join(DATA, "game_armorPieces_v301.dat"), "rb").read()
    piece = lambda i: struct.unpack_from("<4i", raw, i * 64)
    return {"arms": piece(0), "body": piece(1), "legs": piece(2)}


def pc_projectile_names():
    raw = open(os.path.join(DATA, "game_projectile_list.dat"), "rb").read()
    return [raw[i:i + 30].split(b"\0")[0].decode("latin1")
            for i in range(0, len(raw) - PROJ_REC + 1, PROJ_REC)]


def port_ability_tables(port):
    """piece name -> tuple of abilities, from src/player.c."""
    out = {}
    for piece, vals in re.findall(
            r"armor_(arms|body|legs)_ability\[4\] = \{([^}]*)\}", port):
        out[piece] = tuple(int(v) for v in vals.split(","))
    return out


def port_labels(port):
    """{(piece, ability): page text} parsed from dialog_show_armor."""
    block = re.search(r"void dialog_show_armor.*?\n\}", port, re.S).group(0)
    out = {}
    # body / legs: if/else-if chain keyed on the ability value
    for piece, head in (("legs", "THOSE LIGHTER LEGS"),
                        ("body", "THIS FORTIFIED BODY")):
        chain = block.split(head, 1)[1]
        chain = chain[:chain.index("\n    }")]
        for n, label in re.findall(
                r'ability == (\d+)\)\s*lines\[2\] = "([^"]*)"', chain):
            out[(piece, int(n))] = label
        fallback = re.search(r'else\s+lines\[2\] = "([^"]*)"', chain)
        if fallback:
            out[(piece, 1 + max(n for k, n in out if k == piece))] = \
                fallback.group(1)
    # arms: the page is "SHOOT <armor_shot_name(ability)>."
    shot = dict(re.findall(r'case (\d+): return "([^"]*)";',
                           re.search(r"armor_shot_name.*?\n\}", port, re.S)
                           .group(0)))
    if "SHOOT " in block:
        for ability, name in shot.items():
            out[("arms", int(ability))] = "SHOOT %s." % name
    return out


def main():
    data = pc_armor_data()
    defines = read(os.path.join(PC, "defines.h"))
    msg = pc_msg_index(read(os.path.join(PC, "scenes/dialogs.cpp")))
    texts = english_texts(read(os.path.join(PC, "file/fio_strings.cpp")))
    proj_names = pc_projectile_names()
    port = read(PORT_DIALOGS)
    labels = port_labels(port)
    tables = port_ability_tables(read(PORT_PLAYER))

    errors = 0
    for piece, enum in (("legs", "e_ARMOR_ABILITIES_LEGS"),
                        ("body", "e_ARMOR_ABILITIES_BODY"),
                        ("arms", None)):
        order = enum_order(defines, enum) if enum else []
        if piece in tables and tables[piece] != data[piece]:
            print("FAIL  src/player.c armor_%s_ability = %s but the data says %s"
                  % (piece, tables[piece], data[piece]))
            errors += 1
        for player, ability in enumerate(data[piece]):
            tag = "%s player %d (%s) ability %d" % (
                piece, player, PLAYERS[player], ability)
            if piece == "arms":
                want = ("SHOOT %s." % proj_names[ability].upper()
                        if 0 <= ability < len(proj_names) else None)
                if order:
                    tag += " " + order[ability]
            else:
                if ability < 0 or ability >= len(order):
                    continue
                tag += " " + order[ability]
                want = texts.get(msg.get(order[ability], ""), None)
            got = labels.get((piece, ability))
            if want is None:
                print("SKIP  %s - no PC text found" % tag)
                continue
            if got != want:
                print("FAIL  %s\n        data grants: %r\n        page shows:  %r"
                      % (tag, want, got))
                errors += 1
            else:
                print("ok    %s -> %r" % (tag, got))

    print("\n%d mismatched armor page(s)/table(s)" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
