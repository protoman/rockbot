#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Cross-compile Rockbot for the Nintendo GameCube with devkitPPC + libogc and the GameCube SDL 1.2
# ports. It is the Wii port (same code paths, -DWII) with -DGAMECUBE: no Wii remote, no Bluetooth;
# the game data lives on an SD card in an SD Gecko / SD2SP2 adapter (or is loaded by Swiss).
#
# Usage:
#   ./build-gamecube.sh
#
# Output: build-gamecube/rockbot_gamecube.dol
#
# SDK: $WII_SDK (default /mnt/ExtraData/Devel/SDK/Wii/devkitpro-current): the same devkitPro as the
# Wii build (portlibs/gamecube is part of it); build-wii.sh installs it. Optional env: WII_SDK, NCPU.

case "${1:-}" in
    "") ;;
    -h|--help) sed -n '4,17p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "Usage: $0" >&2; exit 1 ;;
esac

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
WII_SDK="${WII_SDK:-/mnt/ExtraData/Devel/SDK/Wii/devkitpro-current}"

if [ ! -x "$WII_SDK/devkitPPC/bin/powerpc-eabi-g++" ] || [ ! -d "$WII_SDK/portlibs/gamecube/lib" ]; then
    echo "Error: devkitPro with the GameCube portlibs not found in $WII_SDK." >&2
    echo "       Run ./build-wii.sh once to install it, or set WII_SDK." >&2
    exit 1
fi

for tool in cmake make pkg-config; do
    command -v "$tool" >/dev/null 2>&1 || { echo "Error: missing host tool: $tool" >&2; exit 1; }
done

export DEVKITPRO="$WII_SDK" DEVKITPPC="$WII_SDK/devkitPPC"
export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"

echo "🛠️ Cross-compiling for the GameCube ($(powerpc-eabi-g++ --version | head -1))..."
cmake -S "$REPO_ROOT" -B "$REPO_ROOT/build-gamecube" \
    -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/GameCube.cmake" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$REPO_ROOT/build-gamecube" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build-gamecube/rockbot_gamecube.dol"
