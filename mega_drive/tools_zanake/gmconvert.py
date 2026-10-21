#!/usr/bin/env python3
"""Convert GameMaker sprites into Mega Drive resources for SGDK's rescomp.

Reads a manifest (res/assets.json) and the sprite folders of the Spelunky
Classic HD project, and writes:

  res/gfx/*.png   indexed images, one 16-color palette per manifest palette
  res/gfx.res     rescomp resource definitions
  res/gfx_ids.h   animation row and tile numbers

In the manifest a sprite is given by name, or as an object with "source" and
"name", plus "region": [x, y, width, height] to use only part of its image,
"stack": N to repeat it N times downwards, "frame": N to use only that one
of its frames, "rotate": N to replace its first frame by N copies turned in
equal steps from 0 to 90 degrees anticlockwise, or "fade": N for N copies
thinned out step by step with a dither, the hardware having no blending.

A sheet of type "picture" is put together from "layers", each a sprite with
"x" and "y" for where its origin goes, drawn in order and cut to the sheet's
"size". rescomp stores it as an image: tiles, with repeats left out, and a map.

A sheet with "borrow": true does not take part in choosing its palette's
colors; its art is drawn with the closest of the colors chosen without it.

The manifest's "shades" gives some colors of the original their Mega Drive
color by hand, as [[r, g, b], [level, level, level]] pairs, for every sprite.
The original's walls and what is carved into them are drawn in shades darker
than the console's first step of brightness, so the nearest color would turn
them all black.

Only the standard library is used. Run with "python3 -I" because the input
comes from another repository.

usage: gmconvert.py MANIFEST SPRITES_DIR RES_DIR
"""

import json
import math
import os
import re
import struct
import sys
import zlib

MAX_COLORS = 15  # index 0 is transparent
TRANSPARENT_RGB = (255, 0, 255)
TILE = 8


class ConvertError(Exception):
    pass


def read_png_rgba(path):
    """Return (width, height, rows) for an 8-bit RGBA, non-interlaced PNG."""
    data = open(path, 'rb').read()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        raise ConvertError('%s is not a PNG' % path)
    pos = 8
    idat = b''
    header = None
    while pos < len(data):
        length, kind = struct.unpack('>I4s', data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        pos += 12 + length
        if kind == b'IHDR':
            header = struct.unpack('>IIBBBBB', body)
        elif kind == b'IDAT':
            idat += body
    if header is None:
        raise ConvertError('%s has no header' % path)
    width, height, depth, color_type, _, _, interlace = header
    if depth != 8 or color_type != 6 or interlace != 0:
        raise ConvertError('%s: only 8-bit RGBA non-interlaced PNGs are supported' % path)

    raw = zlib.decompress(idat)
    stride = width * 4
    previous = bytearray(stride)
    rows = []
    pos = 0
    for _ in range(height):
        filter_type = raw[pos]
        line = bytearray(raw[pos + 1:pos + 1 + stride])
        pos += 1 + stride
        for i in range(stride):
            left = line[i - 4] if i >= 4 else 0
            up = previous[i]
            up_left = previous[i - 4] if i >= 4 else 0
            if filter_type == 1:
                line[i] = (line[i] + left) & 255
            elif filter_type == 2:
                line[i] = (line[i] + up) & 255
            elif filter_type == 3:
                line[i] = (line[i] + ((left + up) >> 1)) & 255
            elif filter_type == 4:
                pa = abs(up - up_left)
                pb = abs(left - up_left)
                pc = abs(left + up - 2 * up_left)
                if pa <= pb and pa <= pc:
                    predictor = left
                elif pb <= pc:
                    predictor = up
                else:
                    predictor = up_left
                line[i] = (line[i] + predictor) & 255
        rows.append(line)
        previous = line
    return width, height, rows


def write_png_indexed(path, width, height, pixels, palette):
    """Write an 8-bit indexed PNG. pixels is a list of rows of palette indexes."""
    def chunk(kind, body):
        crc = zlib.crc32(kind + body) & 0xFFFFFFFF
        return struct.pack('>I', len(body)) + kind + body + struct.pack('>I', crc)

    raw = b''.join(b'\x00' + bytes(row) for row in pixels)
    plte = b''.join(bytes(color) for color in palette)
    with open(path, 'wb') as out:
        out.write(b'\x89PNG\r\n\x1a\n')
        out.write(chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 3, 0, 0, 0)))
        out.write(chunk(b'PLTE', plte))
        out.write(chunk(b'IDAT', zlib.compress(raw, 9)))
        out.write(chunk(b'IEND', b''))


# Brightness the console actually outputs for each of the 8 channel levels.
# The steps are not even, so matching against a linear scale shifts the hue of
# dark colors: a dark brown turns olive.
MD_RAMP = (0, 52, 87, 116, 144, 172, 206, 255)

# Original colors given by hand, from the manifest's "shades".
shades = {}


def md_level(value):
    """The channel level whose real brightness is closest to an 8-bit value."""
    return min(range(len(MD_RAMP)), key=lambda level: abs(MD_RAMP[level] - value))


def md_to_rgb(color):
    """A PNG color that rescomp maps back to exactly this Mega Drive color."""
    return tuple(level * 255 // 7 for level in color)


def load_yy(path):
    """GameMaker .yy files are JSON with trailing commas."""
    text = open(path, encoding='utf-8').read()
    return json.loads(re.sub(r',(\s*[}\]])', r'\1', text))


class Sprite:
    """A GameMaker sprite: frames as rows of Mega Drive colors, None if transparent."""

    def __init__(self, sprites_dir, name, region=None, label=None, stack=1, only_frame=None, rotate=None,
                 fade=None):
        """region is (x, y, width, height) to take only part of each frame;
        stack repeats each frame that many times downwards; only_frame keeps
        just that frame; label names the result in the generated header."""
        folder = os.path.join(sprites_dir, name)
        yy_path = os.path.join(folder, name + '.yy')
        if not os.path.isfile(yy_path):
            raise ConvertError('sprite %s not found in %s' % (name, sprites_dir))
        info = load_yy(yy_path)
        self.name = label or name
        # Two sprites with the same art count once in a palette.
        self.art = (name, region, only_frame, rotate, fade)
        self.width = info['width']
        self.height = info['height']
        self.origin = (info['sequence']['xorigin'], info['sequence']['yorigin'])
        self.frames = []
        for frame in info['frames']:
            width, height, rows = read_png_rgba(os.path.join(folder, frame['name'] + '.png'))
            if (width, height) != (self.width, self.height):
                raise ConvertError('%s: frame size differs from sprite size' % name)
            pixels = []
            for row in rows:
                line = []
                for x in range(width):
                    r, g, b, a = row[x * 4:x * 4 + 4]
                    if a < 128:
                        line.append(None)
                    else:
                        line.append(shades.get((r, g, b)) or (md_level(r), md_level(g), md_level(b)))
                pixels.append(line)
            self.frames.append(pixels)

        if only_frame is not None:
            if only_frame >= len(self.frames):
                raise ConvertError('%s has no frame %d' % (name, only_frame))
            self.frames = [self.frames[only_frame]]

        if region is not None:
            left, top, width, height = region
            if left + width > self.width or top + height > self.height:
                raise ConvertError('%s: region %s is outside the %dx%d sprite'
                                   % (name, region, self.width, self.height))
            self.frames = [[line[left:left + width] for line in frame[top:top + height]]
                           for frame in self.frames]
            self.width = width
            self.height = height

        if stack > 1:
            self.frames = [frame * stack for frame in self.frames]
            self.height *= stack

        if rotate:
            self.frames = [turned(self.frames[0], 90.0 * i / (rotate - 1)) for i in range(rotate)]
        if fade:
            self.frames = [thinned(self.frames[0], (fade - i) / fade) for i in range(fade)]


def turned(frame, degrees):
    """A frame turned anticlockwise about its middle, by nearest pixel."""
    height = len(frame)
    width = len(frame[0])
    centre_x = (width - 1) / 2
    centre_y = (height - 1) / 2
    cos = math.cos(math.radians(degrees))
    sin = math.sin(math.radians(degrees))
    result = []
    for y in range(height):
        line = []
        for x in range(width):
            dx = x - centre_x
            dy = y - centre_y
            source_x = int(round(centre_x + dx * cos - dy * sin))
            source_y = int(round(centre_y + dx * sin + dy * cos))
            inside = 0 <= source_x < width and 0 <= source_y < height
            line.append(frame[source_y][source_x] if inside else None)
        result.append(line)
    return result


BAYER = ((0, 8, 2, 10), (12, 4, 14, 6), (3, 11, 1, 9), (15, 7, 13, 5))


def thinned(frame, share):
    """A frame with only that share of its pixels left, by an ordered dither."""
    return [[color if BAYER[y % 4][x % 4] < share * 16 else None for x, color in enumerate(line)]
            for y, line in enumerate(frame)]


def build_palette(name, sprites, keep=()):
    """Pick at most 15 colors for a group of sprites.

    The colors in keep take no part in merging, neither given up nor taking
    others in: they are the grays of the walls, which the art of several
    palettes has to show alike, and a nearby shade of other art, as the brown
    of a crate, should not turn gray.

    Returns (colors, remap). When the group uses more, the closest pairs are
    merged, the less important color giving way to the more important one.

    A color's importance is the share of each sprite it covers, added up over
    the sprites. Counting plain pixels instead would let a large sprite's
    minor shade outweigh the only blue of a four-pixel gem.
    """
    weight = {}
    pixels = {}
    counted = set()
    for sprite in sprites:
        if sprite.art in counted:
            continue
        counted.add(sprite.art)
        counts = {}
        for frame in sprite.frames:
            for line in frame:
                for color in line:
                    if color is not None:
                        counts[color] = counts.get(color, 0) + 1
        total = sum(counts.values())
        for color, count in counts.items():
            weight[color] = weight.get(color, 0.0) + count / total
            pixels[color] = pixels.get(color, 0) + count

    for color in keep:
        weight.setdefault(color, 0.0)
        pixels.setdefault(color, 0)

    remap = {color: color for color in weight}
    merged = []
    while len(weight) > MAX_COLORS:
        best = None
        colors = sorted(weight)
        for i, a in enumerate(colors):
            for b in colors[i + 1:]:
                distance = sum((MD_RAMP[a[k]] - MD_RAMP[b[k]]) ** 2 for k in range(3))
                cost = distance * min(weight[a], weight[b])
                if a in keep or b in keep:
                    continue
                if best is None or cost < best[0]:
                    best = (cost, a, b)
        _, a, b = best
        stay, drop = (a, b) if weight[a] >= weight[b] else (b, a)
        merged.append((drop, stay, pixels[drop]))
        weight[stay] += weight.pop(drop)
        pixels[stay] += pixels.pop(drop)
        for source, target in remap.items():
            if target == drop:
                remap[source] = stay

    for drop, stay, count in merged:
        print('  %s: merged color %s into %s (%d pixels)' % (name, drop, stay, count))

    # Darkest first, so the order is stable between runs.
    ordered = sorted(weight, key=lambda c: (sum(c), c))
    return ordered, remap


def layer_sprite(layer):
    """The sprite entry of a picture's layer: the layer without its place."""
    entry = {key: value for key, value in layer.items() if key not in ('x', 'y')}
    entry.setdefault('name', entry['source'])
    return entry if len(entry) > 2 else entry['source']


def blank(width, height):
    return [[0] * width for _ in range(height)]


def blit(target, frame, left, top, index_of):
    for y, line in enumerate(frame):
        for x, color in enumerate(line):
            if color is not None:
                target[top + y][left + x] = index_of[color]


def identifier(text):
    text = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', text)
    text = re.sub(r'([A-Z])([A-Z][a-z])', r'\1_\2', text)
    return re.sub(r'[^A-Z0-9]+', '_', text.upper()).strip('_')


def short_name(sprite_name):
    """sRunLeft -> RUN_LEFT, bgCave -> CAVE."""
    return identifier(re.sub(r'^(s|bg)(?=[A-Z])', '', sprite_name))


def convert(manifest_path, sprites_dir, res_dir):
    manifest = json.load(open(manifest_path, encoding='utf-8'))
    for source, target in manifest.get('shades', []):
        shades[tuple(source)] = tuple(target)
    gfx_dir = os.path.join(res_dir, 'gfx')
    os.makedirs(gfx_dir, exist_ok=True)

    res_lines = ['// Generated by tools/gmconvert.py from res/assets.json. Do not edit.', '']
    header = ['// Generated by tools/gmconvert.py from res/assets.json. Do not edit.',
              '#ifndef GFX_IDS_H', '#define GFX_IDS_H', '']

    for group in manifest['palettes']:
        cache = {}

        def sprite(entry):
            """entry is a sprite name, or {"source", "name"} with "region" for
            part of the image, "stack" for several copies one under another,
            "frame" for one frame of several, "rotate" or "fade" for copies of
            it turned or thinned out."""
            if isinstance(entry, str):
                key = (entry, None, None, 1, None, None, None)
            else:
                region = tuple(entry['region']) if 'region' in entry else None
                key = (entry['source'], region, entry['name'], entry.get('stack', 1), entry.get('frame'),
                       entry.get('rotate'), entry.get('fade'))
            if key not in cache:
                cache[key] = Sprite(sprites_dir, *key)
            return cache[key]

        # "keep" lists colors the palette must have; see build_palette.
        # "reserve" lists sprites that will join this palette later. Counting
        # them now keeps the palette, and the look of everything already
        # drawn with it, from changing when they are added.
        entries = list(group.get('reserve', []))
        borrowed = []
        for sheet in group['sheets']:
            sheet_entries = sheet.get('rows', []) + sheet.get('tiles', [])
            if 'source' in sheet:
                sheet_entries.append(sheet['source'])
            # A picture is put together from several sprites.
            sheet_entries += [layer_sprite(layer) for layer in sheet.get('layers', [])]
            # A sheet marked "borrow" has no say in the palette: its art is
            # drawn with the closest colors the others chose. That is how art
            # is added to a palette whose look is already settled.
            if sheet.get('borrow'):
                borrowed += sheet_entries
            else:
                entries += sheet_entries
        colors, remap = build_palette(group['name'], [sprite(e) for e in entries],
                                      [tuple(color) for color in group.get('keep', [])])
        for entry in borrowed:
            for frame in sprite(entry).frames:
                for line in frame:
                    for color in line:
                        if color is not None and color not in remap:
                            remap[color] = min(colors, key=lambda c: sum(
                                (MD_RAMP[c[k]] - MD_RAMP[color[k]]) ** 2 for k in range(3)))
                            print('  %s: borrowed %s for %s' % (group['name'], remap[color], color))
        # Colors fill the palette from the top, so the brightest one is always
        # index 15, which is the index SGDK's font draws with.
        first = 16 - len(colors)
        index_of = {source: first + colors.index(target) for source, target in remap.items()}
        palette = [TRANSPARENT_RGB] + [(0, 0, 0)] * (first - 1) + [md_to_rgb(c) for c in colors]
        print('%s: %d colors' % (group['name'], len(colors)))

        # A one-row swatch image carries the palette for rescomp's PALETTE resource.
        # Its index 0 is black, not the marker color: the console shows that
        # entry of PAL0 wherever nothing is drawn.
        swatch = group['name'] + '.png'
        write_png_indexed(os.path.join(gfx_dir, swatch), 16, 1, [list(range(16))],
                          [(0, 0, 0)] + palette[1:])
        res_lines.append('PALETTE %s "gfx/%s"' % (group['name'], swatch))

        group_index_of = index_of
        for sheet in group['sheets']:
            file_name = sheet['name'] + '.png'
            path = os.path.join(gfx_dir, file_name)
            kind = sheet['type']
            # "recolor" gives some of a borrowing sheet's colors by hand:
            # [[from], [to]] pairs, each "to" one of the palette's colors.
            # The nearest color can be the wrong one, as gold that comes
            # out orange.
            index_of = dict(group_index_of)
            for source, target in sheet.get('recolor', []):
                source, target = tuple(source), tuple(target)
                if target not in colors:
                    raise ConvertError('%s: %s is not in the palette' % (sheet['name'], target))
                index_of[source] = first + colors.index(target)

            if kind == 'sprite':
                cell_w, cell_h = sheet['cell']
                anchor_x, anchor_y = sheet['anchor']
                if cell_w % TILE or cell_h % TILE:
                    raise ConvertError('%s: cell size must be a multiple of 8' % sheet['name'])
                rows = [sprite(n) for n in sheet['rows']]
                columns = max(len(s.frames) for s in rows)
                image = blank(cell_w * columns, cell_h * len(rows))
                prefix = identifier(sheet['name'])
                header.append('// Animations of %s, one per sheet row.' % sheet['name'])
                header.append('enum {')
                for row, s in enumerate(rows):
                    left = anchor_x - s.origin[0]
                    top = anchor_y - s.origin[1]
                    if left < 0 or top < 0 or left + s.width > cell_w or top + s.height > cell_h:
                        raise ConvertError('%s: %s (%dx%d, origin %s) does not fit a %dx%d cell anchored at %s'
                                           % (sheet['name'], s.name, s.width, s.height, s.origin,
                                              cell_w, cell_h, sheet['anchor']))
                    for column, frame in enumerate(s.frames):
                        blit(image, frame, column * cell_w + left, row * cell_h + top, index_of)
                    header.append('    %s_ANIM_%s, // %d frames' % (prefix, short_name(s.name), len(s.frames)))
                header.append('    %s_ANIM_COUNT' % prefix)
                header.append('};')
                header.append('// Frames in each animation, for an array initializer.')
                header.append('#define %s_ANIM_FRAMES { %s }' % (prefix, ', '.join(str(len(s.frames)) for s in rows)))
                header.append('#define %s_CELL_WIDTH %d' % (prefix, cell_w))
                header.append('#define %s_ANCHOR_X %d' % (prefix, anchor_x))
                header.append('#define %s_ANCHOR_Y %d' % (prefix, anchor_y))
                header.append('')
                write_png_indexed(path, cell_w * columns, cell_h * len(rows), image, palette)
                res_lines.append('SPRITE %s "gfx/%s" %d %d NONE %d'
                                 % (sheet['name'], file_name, cell_w // TILE, cell_h // TILE,
                                    sheet.get('time', 0)))

            elif kind == 'tiles':
                size = sheet['size']
                if size % TILE:
                    raise ConvertError('%s: tile size must be a multiple of 8' % sheet['name'])
                tiles = [sprite(n) for n in sheet['tiles']]
                image = blank(size * len(tiles), size)
                prefix = identifier(sheet['name'])
                header.append('// Blocks of %s, %dx%d pixels each, left to right in the sheet.' % (sheet['name'], size, size))
                header.append('enum {')
                for column, s in enumerate(tiles):
                    if (s.width, s.height) != (size, size):
                        raise ConvertError('%s: %s is %dx%d, expected %dx%d'
                                           % (sheet['name'], s.name, s.width, s.height, size, size))
                    blit(image, s.frames[0], column * size, 0, index_of)
                    header.append('    %s_%s,' % (prefix, short_name(s.name)))
                header.append('    %s_COUNT' % prefix)
                header.append('};')
                header.append('')
                write_png_indexed(path, size * len(tiles), size, image, palette)
                # No tile optimization: the code relies on the sheet layout.
                res_lines.append('TILESET %s "gfx/%s" NONE NONE' % (sheet['name'], file_name))

            elif kind == 'image':
                s = sprite(sheet['source'])
                if s.width % TILE or s.height % TILE:
                    raise ConvertError('%s: %s size must be a multiple of 8' % (sheet['name'], s.name))
                image = blank(s.width, s.height)
                blit(image, s.frames[0], 0, 0, index_of)
                prefix = identifier(sheet['name'])
                header.append('#define %s_WIDTH_TILES %d' % (prefix, s.width // TILE))
                header.append('#define %s_HEIGHT_TILES %d' % (prefix, s.height // TILE))
                header.append('')
                write_png_indexed(path, s.width, s.height, image, palette)
                res_lines.append('TILESET %s "gfx/%s" NONE NONE' % (sheet['name'], file_name))

            elif kind == 'picture':
                # Several sprites drawn over one another, each with its
                # origin at a point, and cut to the picture's size. rescomp
                # stores it as a tile map without repeated tiles.
                width, height = sheet['size']
                if width % TILE or height % TILE:
                    raise ConvertError('%s: size must be a multiple of 8' % sheet['name'])
                image = blank(width, height)
                for layer in sheet['layers']:
                    s = sprite(layer_sprite(layer))
                    left = layer['x'] - s.origin[0]
                    top = layer['y'] - s.origin[1]
                    for y, line in enumerate(s.frames[0]):
                        for x, color in enumerate(line):
                            if color is not None and 0 <= left + x < width and 0 <= top + y < height:
                                image[top + y][left + x] = index_of[color]
                write_png_indexed(path, width, height, image, palette)
                res_lines.append('IMAGE %s "gfx/%s" BEST ALL' % (sheet['name'], file_name))

            else:
                raise ConvertError('%s: unknown sheet type %s' % (sheet['name'], kind))

        res_lines.append('')

    header += ['#endif', '']
    with open(os.path.join(res_dir, 'gfx.res'), 'w', encoding='utf-8') as out:
        out.write('\n'.join(res_lines))
    with open(os.path.join(res_dir, 'gfx_ids.h'), 'w', encoding='utf-8') as out:
        out.write('\n'.join(header))


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    try:
        convert(sys.argv[1], sys.argv[2], sys.argv[3])
    except ConvertError as error:
        sys.exit('gmconvert: %s' % error)


if __name__ == '__main__':
    main()
