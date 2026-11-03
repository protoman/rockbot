#!/usr/bin/env bash
# Build and package RockBot for the PlayStation 3 (PSL1GHT).
#
# Usage:
#   ports/ps3/build.sh [1|2] [--no-package] [--pkg]
#
#   1|2           which game to package (default 1): build/games/RockDroid<N>
#   --no-package  only compile
#   --pkg         also create the installable .pkg (for a PS3 running custom
#                 firmware / HEN, "Install Package Files")
#
# Outputs:
#   build-ps3/PS3_GAME/                folder RPCS3 (and a CFW PS3 via /app_home) can boot
#   build-ps3/Rockbot<N>.pkg           installer (only with --pkg)
#
# PS3DEV must point at a ps3dev installation (default
# /mnt/ExtraData/Devel/SDK/PS3/ps3dev). The nightly all-in-one release from
# https://github.com/ps3dev/ps3dev/releases includes the PPU toolchain,
# PSL1GHT and the SDL 1.2 portlibs.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PS3DEV="${PS3DEV:-/mnt/ExtraData/Devel/SDK/PS3/ps3dev}"
export PS3DEV
export PSL1GHT="$PS3DEV"

GAME_NUMBER=1
PACKAGE=1
MAKE_PKG=0
for arg in "$@"; do
    case "$arg" in
        1|2) GAME_NUMBER="$arg" ;;
        --no-package) PACKAGE=0 ;;
        --pkg) MAKE_PKG=1 ;;
        *) echo "Usage: $0 [1|2] [--no-package] [--pkg]" >&2; exit 1 ;;
    esac
done

BUILD="$ROOT/build-ps3"
STAGE="$BUILD/PS3_GAME"
GAME_DATA="$ROOT/build/games/RockDroid${GAME_NUMBER}"
# PS3 title ids are exactly 9 characters.
APPID="ROCKBOT0${GAME_NUMBER}"
TITLE="RockBot ${GAME_NUMBER}"

if [ ! -x "$PS3DEV/ppu/bin/ppu-g++" ]; then
    echo "error: PPU toolchain not found under $PS3DEV" >&2
    echo "       set PS3DEV to your ps3dev installation" >&2
    exit 1
fi

# file/version.h <- build/packages/version_name.txt
"$ROOT/build/packages/sync_version.sh"

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchain-ps3.cmake" \
    -DPS3DEV="$PS3DEV" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD" -j"$(nproc)"

if [ "$PACKAGE" = 0 ]; then
    exit 0
fi

if [ ! -d "$GAME_DATA" ]; then
    echo "error: game data not found: $GAME_DATA" >&2
    exit 1
fi

# --- Package a folder RPCS3 (and a CFW PS3 via /app_home) can boot ----------
rm -rf "$STAGE"
mkdir -p "$STAGE/USRDIR/games"

"$PS3DEV/ppu/bin/ppu-strip" --strip-all "$BUILD/rockbot" -o "$BUILD/rockbot_stripped"
cp "$BUILD/rockbot_stripped" "$BUILD/rockbot_linked"
"$PS3DEV/bin/sprxlinker" "$BUILD/rockbot_linked"
"$PS3DEV/bin/fself" "$BUILD/rockbot_linked" "$STAGE/USRDIR/EBOOT.BIN"

# The game is started without menus when only one game folder is present.
cp -r "$GAME_DATA" "$STAGE/USRDIR/games/"
cp -r "$ROOT/build/shared" "$STAGE/USRDIR/"
cp -r "$ROOT/build/fonts" "$STAGE/USRDIR/"
# editor/debug leftovers and saves must not ship
find "$STAGE/USRDIR" \( -name '.svn' -o -name 'errors.log' -o -name '*.sav' \) -prune -exec rm -rf {} +

# SDL_image on PS3 fails on the game's single grayscale PNG ("Out of memory").
# Convert any grayscale PNG in the packaged data to RGB.
if command -v convert >/dev/null 2>&1; then
    find "$STAGE/USRDIR" -name '*.png' -print0 | while IFS= read -r -d '' png; do
        if file "$png" | grep -q 'grayscale'; then
            convert "$png" -type TrueColor "PNG24:$png.tmp" && mv "$png.tmp" "$png"
        fi
    done
fi

# PARAM.SFO: title, id and a MM.NN version taken from build/packages/version_name.txt
VERSION_FILE="$ROOT/build/packages/version_name.txt"
APP_VER="01.00"
if [ -f "$VERSION_FILE" ]; then
    IFS=. read -r v_major v_minor _ <<< "$(tr -d '[:space:]' < "$VERSION_FILE")"
    APP_VER="$(printf '%02d.%02d' "$((10#$v_major))" "$((10#$v_minor))")"
fi
SFO_XML="$BUILD/param_sfo_${GAME_NUMBER}.xml"
python3 - "$PS3DEV/bin/sfo.xml" "$SFO_XML" "$APP_VER" <<'PYEOF'
import re, sys
src, dst, ver = sys.argv[1:4]
s = open(src).read()
for name in ("APP_VER", "VERSION"):
    s = re.sub(r'(<value name="%s" type="string">\s*)[^<]*?(\s*</value>)' % name, r'\g<1>%s\g<2>' % ver, s)
open(dst, "w").write(s)
PYEOF
"$PS3DEV/bin/sfo" -f "$SFO_XML" --title="$TITLE" --appid="$APPID" "$STAGE/PARAM.SFO"

# XMB icon (320x176). Built from the game logo when ImageMagick is available.
if command -v convert >/dev/null 2>&1 && [ -f "$GAME_DATA/images/logo.png" ]; then
    convert "$GAME_DATA/images/logo.png" -background '#001533' -resize 320x176 \
        -gravity center -extent 320x176 "PNG24:$STAGE/ICON0.PNG" 2>/dev/null \
        || cp "$PS3DEV/bin/ICON0.PNG" "$STAGE/ICON0.PNG"
else
    cp "$PS3DEV/bin/ICON0.PNG" "$STAGE/ICON0.PNG"
fi

echo "PS3 package folder: $STAGE"
echo "Boot with: rpcs3 $STAGE/USRDIR/EBOOT.BIN"

# --- Installable .pkg (PSL1GHT rules, same as ppu_rules) ---------------------
if [ "$MAKE_PKG" = 1 ]; then
    CONTENTID="UP0001-${APPID}_00-0000000000000000"
    PKGROOT="$BUILD/pkg_${GAME_NUMBER}"
    PKG_OUT="$BUILD/Rockbot${GAME_NUMBER}.pkg"

    rm -rf "$PKGROOT" "$PKG_OUT"
    mkdir -p "$PKGROOT/USRDIR"
    cp -r "$STAGE/USRDIR/." "$PKGROOT/USRDIR/"
    cp "$STAGE/PARAM.SFO" "$STAGE/ICON0.PNG" "$PKGROOT/"

    # The ps3dev `pkg` tool silently drops everything past 1024 entries.
    PKG_ENTRIES="$(find "$PKGROOT" -mindepth 1 | wc -l)"
    if [ "$PKG_ENTRIES" -gt 1000 ]; then
        echo "error: $PKG_ENTRIES files/folders; the ps3dev pkg tool supports at most 1024" >&2
        exit 1
    fi
    # NPDRM self: the output file name matters, do not rename EBOOT.BIN.
    "$PS3DEV/bin/make_self_npdrm" "$BUILD/rockbot_linked" "$PKGROOT/USRDIR/EBOOT.BIN" "$CONTENTID"
    # The pkg tool keeps ~0.5 MB of arrays on the stack per folder level and overflows the
    # default 8 MB stack on deep trees (segfault): give it a bigger stack.
    ( ulimit -s 1048576 2>/dev/null || ulimit -s unlimited 2>/dev/null || true
      "$PS3DEV/bin/pkg" --contentid "$CONTENTID" "$PKGROOT/" "$PKG_OUT" )
    "$PS3DEV/bin/package_finalize" "$PKG_OUT"
    echo "PS3 installer: $PKG_OUT"
fi
