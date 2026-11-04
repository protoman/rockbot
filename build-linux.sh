#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Native Linux build of Rockbot (CMake, no sudo/apt: install the -dev packages yourself,
# see build-ubuntu.sh for the Ubuntu list).
#
# Usage:
#   ./build-linux.sh              → SDL2 (default)
#   ./build-linux.sh sdl2|sdl3
#
# Output: build/rockbot_linux

USE_SDL_VERSION=2
for arg in "$@"; do
    case "$(printf '%s' "$arg" | tr '[:upper:]' '[:lower:]')" in
        sdl2) USE_SDL_VERSION=2 ;;
        sdl3) USE_SDL_VERSION=3 ;;
        -h|--help) sed -n '4,11p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Usage: $0 [sdl2|sdl3]" >&2; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build-linux${ROCKBOT_IN_DOCKER:+-docker}-sdl${USE_SDL_VERSION}"

for tool in cmake make pkg-config g++; do
    command -v "$tool" >/dev/null 2>&1 || { echo "Error: missing host tool: $tool" >&2; exit 1; }
done

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL_VERSION="$USE_SDL_VERSION"
cmake --build "$BUILD_DIR" -j"$(nproc)"

strip --strip-unneeded "$SCRIPT_DIR/build/rockbot_linux" 2>/dev/null || true
echo "Built $SCRIPT_DIR/build/rockbot_linux"
