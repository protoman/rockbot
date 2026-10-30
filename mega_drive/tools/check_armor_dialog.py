#!/usr/bin/env python3
"""Offline check: the "got armor" dialog text must match the ability the data
actually installs.

BUG it reproduces: ROCKBOT gets the legs piece with ability 0 (double jump) but
the page reads "DASH IN MIDDLE-AIR." - the port's dialogs.c keyed the labels
off a different enum order than src/player.c uses.

Source of truth is the PC code, not a table copied here:
  ../defines.h            enum e_ARMOR_ABILITIES_LEGS / _BODY order
  ../scenes/dialogs.cpp   enum value -> message index (msg1..msg4)
  ../file/fio_strings.cpp English text of that message
  data/games/RockDroid1/game_armorPieces_v301.dat  ability per piece/player
and the port's own text in src/dialogs.c is what gets compared.

Usage: tools/check_armor_dialog.py      exit 1 on a mismatch.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
MD = os.path.dirname(HERE)
PC = os.path.dirname(MD)
DATA = os.path.join(MD, "data/games/RockDroid1/game_armorPieces_v301.dat")
PORT_SRC = os.path.join(MD, "src/dialogs.c")
PLAYERS = ("ROCKBOT", "BETABOT", "CANDYBOT", "KITTYBOT")


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


def port_labels(port):
    """Return {(kind, ability): label} parsed from dialog_show_armor."""
    block = re.search(r"void dialog_show_armor.*?\n\}", port, re.S).group(0)
    out = {}
    for kind, head in (("legs", "THOSE LIGHTER LEGS"),
                       ("body", "THIS FORTIFIED BODY")):
        chain = block.split(head, 1)[1]
        chain = chain[:chain.index("\n    }")]     # the page's if/else chain
        for n, label in re.findall(
                r'ability == (\d+)\)\s*lines\[2\] = "([^"]*)"', chain):
            out[(kind, int(n))] = label
        fallback = re.search(r'else\s+lines\[2\] = "([^"]*)"', chain)
        if fallback:
            out[(kind, 1 + max(n for k, n in out if k == kind))] = \
                fallback.group(1)
    return out


def main():
    # st_armor_piece: special_ability[4] then got_message[4][3] -> 64 B stride
    raw = open(DATA, "rb").read()
    piece = lambda i: struct.unpack_from("<4i", raw, i * 64)
    data = {"arms": piece(0), "body": piece(1), "legs": piece(2)}

    defines = read(os.path.join(PC, "defines.h"))
    msg = pc_msg_index(read(os.path.join(PC, "scenes/dialogs.cpp")))
    texts = english_texts(read(os.path.join(PC, "file/fio_strings.cpp")))
    port = port_labels(read(PORT_SRC))

    errors = 0
    for kind, enum in (("legs", "e_ARMOR_ABILITIES_LEGS"),
                       ("body", "e_ARMOR_ABILITIES_BODY")):
        order = enum_order(defines, enum)
        for player, ability in enumerate(data[kind]):
            if ability < 0 or ability >= len(order):
                continue
            want = texts.get(msg.get(order[ability], ""), None)
            got = port.get((kind, ability))
            tag = "%s player %d (%s) ability %d %s" % (
                kind, player, PLAYERS[player], ability, order[ability])
            if want is None:
                print("SKIP  %s - no PC text found" % tag)
                continue
            if got != want:
                print("FAIL  %s\n        data grants: %r\n        page shows:  %r"
                      % (tag, want, got))
                errors += 1
            else:
                print("ok    %s -> %r" % (tag, got))

    print("\n%d mismatched armor page(s)" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
