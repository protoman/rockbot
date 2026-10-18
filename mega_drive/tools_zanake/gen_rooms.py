#!/usr/bin/env python3
"""Extract an area's room templates from the GameMaker source.

Reads scripts/scrRoomGen/scrRoomGen.gml and writes src/rooms_mines.h, or
with "lush" scripts/scrRoomGen2/scrRoomGen2.gml and src/rooms_lush.h, or
with "market" scripts/scrRoomGenMarket/scrRoomGenMarket.gml and
src/rooms_market.h, whose obstacles are the jungle's and are left out, or
with "ice" scripts/scrRoomGen3/scrRoomGen3.gml and src/rooms_ice.h, or
with "yeti" scripts/scrRoomGenYeti/scrRoomGenYeti.gml and src/rooms_yeti.h. The
templates are copied character for character; only their grouping into C
arrays is decided here. Run with "python3 -I".

usage: gen_rooms.py SCR_ROOM_GEN.gml OUT.h [lush|market|ice|yeti]
"""

import re
import sys

# Branches of scrRoomGen, in source order, and the array each one fills.
# Shop rooms (roomPath 4 and 5) are not extracted yet.
BRANCHES = [
    ('startRoomX', 'rooms_start', 8),
    ('endRoomX', 'rooms_end', 6),
    ('roomPath == 0)', 'rooms_side', 11),
    ('roomPath == 0 or roomPath == 1', 'rooms_main', 14),
    ('roomPath == 3', 'rooms_drop_in', 9),
    ('roomPath == 4', None, 3),
    ('roomPath == 5', None, 3),
    ('roomPath == 8', 'rooms_pit', 1),
    ('roomPath == 9', 'rooms_pit_bottom', 1),
    ('else // drop', 'rooms_drop', 12),
]

OBSTACLES = [('obstacles_door', 8), ('obstacles_ground', 16), ('obstacles_air', 10)]

# The same for scrRoomGen2. The idol room and one main room come in two
# versions each; the last obstacles are the vines, which are four rows high.
LUSH_BRANCHES = [
    ('startRoomX', 'lush_rooms_start', 4),
    ('endRoomX', 'lush_rooms_end', 5),
    ('roomPath == 0 and', 'lush_rooms_side', 11),
    ('roomPath == 0 or roomPath == 1', 'lush_rooms_main', 11),
    ('roomPath == 3', 'lush_rooms_drop_in', 7),
    ('roomPath == 4', None, 3),
    ('roomPath == 5', None, 3),
    ('roomPath == 7', 'lush_rooms_lake_bottom', 8),
    ('roomPath == 8', 'lush_rooms_lake_top', 8),
    ('roomPath == 9', 'lush_rooms_jaws', 1),
    ('else // drop', 'lush_rooms_drop', 6),
]

LUSH_OBSTACLES = [('lush_obstacles_door', 1), ('lush_obstacles_ground', 5), ('lush_obstacles_air', 4),
                  ('lush_obstacles_vines', 3)]


# The black market, scrRoomGenMarket: its shops are rooms like the rest.
MARKET_BRANCHES = [
    ('startRoomX', 'market_rooms_start', 4),
    ('endRoomX', 'market_rooms_end', 4),
    ('roomPath == 1', 'market_rooms_main', 8),
    ('roomPath == 3', 'market_rooms_drop_in', 7),
    ('roomPath == 4 and x == 496', 'market_rooms_ankh', 1),
    ('roomPath == 4)', 'market_rooms_shop', 1),
    ('roomPath == 5', 'market_rooms_dice', 1),
    ('else // drop', 'market_rooms_drop', 6),
]


# The ice caves, scrRoomGen3. Their shops are the mines' with another floor.
# The last obstacles are three blocks wide: platforms that fall, and ice.
ICE_BRANCHES = [
    ('startRoomX', 'ice_rooms_start', 2),
    ('endRoomX', 'ice_rooms_end', 1),
    ('roomPath == 0 and', 'ice_rooms_side', 10),
    ('roomPath == 0 or roomPath == 1 or roomPath == 2', 'ice_rooms_main', 9),
    ('roomPath == 4', None, 3),
    ('roomPath == 5', None, 3),
    ('roomPath == 6', 'ice_rooms_moai', 2),
    ('roomPath == 7', 'ice_rooms_craft_left', 1),
    ('roomPath == 8', 'ice_rooms_craft', 1),
    ('roomPath == 9', 'ice_rooms_craft_right', 1),
    ('else', 'ice_rooms_drop', 1),
]

ICE_OBSTACLES = [('ice_obstacles_door', 6), ('ice_obstacles_ground', 15), ('ice_obstacles_air', 4)]


# The yeti lair, scrRoomGenYeti. The third side room is commented out in
# the script, and its two pit rooms are never used.
YETI_BRANCHES = [
    ('startRoomX', 'yeti_rooms_start', 4),
    ('endRoomX', 'yeti_rooms_end', 6),
    ('roomPath == 0 and', 'yeti_rooms_side', 3),
    ('roomPath == 0 or roomPath == 1', 'yeti_rooms_main', 10),
    ('roomPath == 3', 'yeti_rooms_drop_in', 9),
    ('roomPath == 4', None, 3),
    ('roomPath == 5', None, 3),
    ('roomPath == 8', None, 1),
    ('roomPath == 9', None, 1),
    ('else // drop', 'yeti_rooms_drop', 12),
]

YETI_OBSTACLES = [('yeti_obstacles_door', 7), ('yeti_obstacles_ground', 16), ('yeti_obstacles_air', 10)]


def main():
    if len(sys.argv) not in (3, 4) or (len(sys.argv) == 4 and sys.argv[3] not in ('lush', 'market', 'ice', 'yeti')):
        sys.exit(__doc__)
    ice = len(sys.argv) == 4 and sys.argv[3] == 'ice'
    yeti = len(sys.argv) == 4 and sys.argv[3] == 'yeti'
    lush = len(sys.argv) == 4 and sys.argv[3] == 'lush'
    market = len(sys.argv) == 4 and sys.argv[3] == 'market'
    BRANCHES, OBSTACLES = (LUSH_BRANCHES, LUSH_OBSTACLES) if lush else (MARKET_BRANCHES, []) if market \
        else (ICE_BRANCHES, ICE_OBSTACLES) if ice else (YETI_BRANCHES, YETI_OBSTACLES) if yeti \
        else (globals()['BRANCHES'], globals()['OBSTACLES'])
    lines = open(sys.argv[1], encoding='utf-8').read().split('\n')

    groups = []
    branch = 0
    for line in lines:
        text = line.strip()
        if text.startswith('// Add obstacles'):
            break
        if branch < len(BRANCHES) and BRANCHES[branch][0] in text and 'strTemp' not in text:
            groups.append([])
            branch += 1
        # The first assignment in the script is an all-empty default, before any branch.
        for template in re.findall(r'strTemp = "([^"]{80})"', text if branch else ''):
            if groups:
                groups[-1].append(template)

    chunks = re.findall(r'strObs1 = "(\w{5})"; strObs2 = "(\w{5})"; strObs3 = "(\w{5})";(?: strObs4 = "(\w{5})";)?',
                        '\n'.join(lines))

    narrow = re.findall(r'strObs1 = "(\w{3})"; strObs2 = "(\w{3})"; strObs3 = "(\w{3})";', '\n'.join(lines))
    if yeti:
        out = ['// Generated by tools/gen_rooms.py from scrRoomGenYeti.gml. Do not edit.',
               '//',
               '// As rooms_mines.h, which goes first.',
               '#ifndef ROOMS_YETI_H', '#define ROOMS_YETI_H', '']
    elif ice:
        out = ['// Generated by tools/gen_rooms.py from scrRoomGen3.gml. Do not edit.',
               '//',
               '// As rooms_mines.h, which goes first. The last obstacles are 3x3.',
               '#ifndef ROOMS_ICE_H', '#define ROOMS_ICE_H', '']
    elif market:
        chunks = []
        out = ['// Generated by tools/gen_rooms.py from scrRoomGenMarket.gml. Do not edit.',
               '//',
               '// As rooms_mines.h, which goes first.',
               '#ifndef ROOMS_MARKET_H', '#define ROOMS_MARKET_H', '']
    elif lush:
        out = ['// Generated by tools/gen_rooms.py from scrRoomGen2.gml. Do not edit.',
               '//',
               '// As rooms_mines.h, which goes first. The vines are 5x4.',
               '#ifndef ROOMS_LUSH_H', '#define ROOMS_LUSH_H', '']
    else:
        out = ['// Generated by tools/gen_rooms.py from scrRoomGen.gml. Do not edit.',
               '//',
               '// Each room is 10x8 blocks, written row by row. Each obstacle is 5x3.',
               '#ifndef ROOMS_MINES_H', '#define ROOMS_MINES_H', '',
               '#define ROOM_CELLS 80', '#define OBSTACLE_CELLS 15', '']
    for (marker, name, count), templates in zip(BRANCHES, groups):
        if len(templates) != count:
            sys.exit('gen_rooms: expected %d templates for "%s", found %d' % (count, marker, len(templates)))
        if name is None:
            continue
        out.append('static const char %s[%d][ROOM_CELLS + 1] = {' % (name, count))
        out += ['    "%s",' % t for t in templates]
        out += ['};', '']

    if len(chunks) != sum(count for _, count in OBSTACLES):
        sys.exit('gen_rooms: unexpected number of obstacle chunks: %d' % len(chunks))
    start = 0
    for name, count in OBSTACLES:
        tall = bool(chunks[start][3])
        out.append('static const char %s[%d][OBSTACLE_CELLS + %s] = {' % (name, count, '6' if tall else '1'))
        out += ['    "%s",' % ''.join(c) for c in chunks[start:start + count]]
        out += ['};', '']
        start += count

    if ice:
        if len(narrow) != 12:
            sys.exit('gen_rooms: unexpected number of narrow obstacle chunks: %d' % len(narrow))
        out.append('static const char ice_obstacles_falling[12][10] = {')
        out += ['    "%s",' % ''.join(c) for c in narrow]
        out += ['};', '']

    out += ['#endif', '']
    open(sys.argv[2], 'w', encoding='utf-8').write('\n'.join(out))


if __name__ == '__main__':
    main()
