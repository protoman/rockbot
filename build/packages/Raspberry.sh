#!/usr/bin/env bash
# Raspberry Pi tarball (Raspberry Pi OS, SDL2): cross-builds in Docker (see ../../build-raspberry.sh)
# and creates Rockbot<N>_RaspberryPi_<arch>_<version>.tar.gz
# Usage: ./Raspberry.sh [1|2] [armhf|arm64]    (asks which game if omitted; armhf = 32-bit OS)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"

game="" ; arch="armhf"
for arg in "$@"; do
    case "$arg" in
        1|2) game="$arg" ;;
        armhf|arm64) arch="$arg" ;;
        *) echo "Usage: $0 [1|2] [armhf|arm64]" >&2; exit 1 ;;
    esac
done
choose_game "$game" || exit 1

GAME_DIR="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_RaspberryPi_${arch}_$VERSIONNAME.tar.gz"

../../build-raspberry.sh "$arch"

rm -rf ./raspberry
mkdir -p "./raspberry/$GAME_DIR"
cp "../rockbot_pi_$arch" "./raspberry/$GAME_DIR/rockbot"
stage_game_data "./raspberry/$GAME_DIR"
cat > "./raspberry/$GAME_DIR/rockbot.sh" <<'LAUNCHER'
#!/bin/sh
cd "$(dirname "$(readlink -f "$0")")" && exec ./rockbot "$@"
LAUNCHER
chmod +x "./raspberry/$GAME_DIR/rockbot.sh" "./raspberry/$GAME_DIR/rockbot"
cat > "./raspberry/$GAME_DIR/README.TXT" <<README
Rockbot $version_number $VERSIONNAME (Raspberry Pi OS, $arch)

Run:   ./rockbot.sh
Needs: sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-mixer-2.0-0 libsdl2-ttf-2.0-0 libsdl2-gfx-1.0-0
Built on Debian bookworm: Raspberry Pi OS bookworm or newer.
Saves and settings are stored in ~/.rockdroid/
README

rm -f "$OUT"
tar -czf "$OUT" -C ./raspberry "$GAME_DIR"
echo "Package: $OUT"
