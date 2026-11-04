#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Cross-compile Rockbot for the Nintendo Wii with the current devkitPro devkitPPC + libogc and the
# Wii SDL 1.2 ports, natively (no Docker needed to build).
#
# Usage:
#   ./build-wii.sh
#
# Output: build-wii/rockbot_wii.dol
#
# SDK: $WII_SDK (default /mnt/ExtraData/Devel/SDK/Wii/devkitpro-current). If it is missing it is
# installed once by copying /opt/devkitpro out of the official devkitpro/devkitppc Docker image
# (Docker is only used for that copy) and relocating its paths. Optional env: WII_SDK, NCPU.

case "${1:-}" in
    "") ;;
    -h|--help) sed -n '4,16p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "Usage: $0" >&2; exit 1 ;;
esac

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
WII_SDK="${WII_SDK:-/mnt/ExtraData/Devel/SDK/Wii/devkitpro-current}"

if [ ! -x "$WII_SDK/devkitPPC/bin/powerpc-eabi-g++" ]; then
    command -v docker >/dev/null 2>&1 || {
        echo "Error: no devkitPro in $WII_SDK and docker is not available to install it." >&2
        echo "       Install devkitPro (devkitPPC, libogc, wii-sdl* ports) and set WII_SDK to its folder." >&2
        exit 1
    }
    echo "🛠️ Installing devkitPro into $WII_SDK (one time, from the devkitpro/devkitppc image)..."
    mkdir -p "$WII_SDK"
    docker run --rm devkitpro/devkitppc:latest tar -C /opt/devkitpro \
        --exclude=./pacman --exclude=./devkitARM --exclude=./devkitA64 --exclude=./wut --exclude=./examples \
        -cf - . | tar -xf - -C "$WII_SDK"
    # The SDK hard-codes /opt/devkitpro in scripts and .pc files: point them at the new location.
    grep -rIl "/opt/devkitpro" "$WII_SDK" 2>/dev/null | xargs -r sed -i "s|/opt/devkitpro|$WII_SDK|g"
fi

for tool in cmake make pkg-config; do
    command -v "$tool" >/dev/null 2>&1 || { echo "Error: missing host tool: $tool" >&2; exit 1; }
done

export DEVKITPRO="$WII_SDK" DEVKITPPC="$WII_SDK/devkitPPC"
export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"

echo "🛠️ Cross-compiling for the Wii ($(powerpc-eabi-g++ --version | head -1))..."
cmake -S "$REPO_ROOT" -B "$REPO_ROOT/build-wii" \
    -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Wii.cmake" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$REPO_ROOT/build-wii" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build-wii/rockbot_wii.dol"
