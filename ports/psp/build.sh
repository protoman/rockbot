#!/usr/bin/env bash
# Build and package RockBot for the PSP (pspdev, CMake).
#
# Usage:
#   ports/psp/build.sh [1|2] [--no-package]
#
# ROCKBOT_PSP_POISON=ON fills every C++ allocation with 0xCD (real PSP memory is not zeroed, the
# emulator's is): finds reads of uninitialised members that only break on hardware.
#
# PSPDEV must point at a pspdev installation (default
# /mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev). Prebuilt releases:
# https://github.com/pspdev/pspdev/releases (pspdev-ubuntu-latest-x86_64.tar.gz)

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export PSPDEV="${PSPDEV:-/mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev}"
export PATH="$PSPDEV/bin:$PATH"

VERSION_NUMBER=1
PACKAGE=1
for arg in "$@"; do
    case "$arg" in
        1|2) VERSION_NUMBER="$arg" ;;
        --no-package) PACKAGE=0 ;;
        *) echo "Usage: $0 [1|2] [--no-package]" >&2; exit 1 ;;
    esac
done

"$ROOT/build/packages/sync_version.sh"   # file/version.h <- version_name.txt

BUILD="$ROOT/res/psp"
PACKAGES_DIR="$ROOT/build/packages"
VERSION_NAME="0.0.0"
if [ -f "$PACKAGES_DIR/version_name.txt" ]; then
    VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name.txt")"
fi

if [ ! -x "$PSPDEV/bin/psp-g++" ]; then
    echo "error: PSP toolchain not found under $PSPDEV (set PSPDEV)" >&2
    exit 1
fi

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$PSPDEV/psp/share/pspdev.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DROCKBOT_PSP_POISON="${ROCKBOT_PSP_POISON:-OFF}" \
    -DBUILD_PRX=ON \
    -DROCKBOT_VERSION_NUMBER="$VERSION_NUMBER" \
    -DROCKBOT_VERSION_NAME="$VERSION_NAME"

cmake --build "$BUILD" -j"$(nproc)"

[ "$PACKAGE" = 1 ] || exit 0

GAME_DIR="Rockbot${VERSION_NUMBER}"
STAGE="$PACKAGES_DIR/psp_dist/$GAME_DIR"
rm -rf "$STAGE"
mkdir -p "$STAGE/games"
cp "$BUILD/EBOOT.PBP" "$STAGE/"
cp -r "$ROOT/build/fonts" "$ROOT/build/shared" "$STAGE/"
cp -r "$ROOT/build/games/Rockbot${VERSION_NUMBER}" "$STAGE/games/"
rm -rf "$STAGE/games/Rockbot${VERSION_NUMBER}/mp3"

echo "PSP package: $STAGE"
echo "Copy it to ms0:/PSP/GAME/ (or into PPSSPP's PSP/GAME)."
