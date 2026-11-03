#!/bin/bash
set -e
set -o pipefail

# Usage:
#   ./build-ps2.sh
#
# Optional env:
#   PS2DEV   ps2dev install root (default: /mnt/ExtraData/Devel/SDK/PS2.new/ps2dev)
#   NCPU     make parallelism (default: nproc)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
export PS2DEV="${PS2DEV:-/mnt/ExtraData/Devel/SDK/PS2.new/ps2dev}"
export PS2SDK="$PS2DEV/ps2sdk"
export GSKIT="$PS2DEV/gsKit"
export PATH="$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2SDK/bin:$PATH"
BUILD_DIR="$REPO_ROOT/build-ps2"
NCPU="${NCPU:-$(nproc)}"

if [ ! -x "$PS2DEV/ee/bin/mips64r5900el-ps2-elf-g++" ]; then
    echo "PS2 toolchain not found at $PS2DEV" >&2
    echo "Set PS2DEV to the ps2dev install root (contains ee/, iop/, ps2sdk/, gsKit/)." >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$PS2DEV/share/ps2dev.cmake" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$NCPU"

echo "Built $REPO_ROOT/build/rockbot.elf"
