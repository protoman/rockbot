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
PS2DEV="${PS2DEV:-/mnt/ExtraData/Devel/SDK/PS2.new/ps2dev}"
BUILD_DIR="$REPO_ROOT/build-ps2"
NCPU="${NCPU:-$(nproc)}"

if [ ! -x "$PS2DEV/ee/bin/mips64r5900el-ps2-elf-g++" ]; then
    echo "PS2 toolchain not found at $PS2DEV" >&2
    echo "Set PS2DEV to the ps2dev install root (contains ee/, iop/, ps2sdk/, gsKit/)." >&2
    exit 1
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

PS2DEV="$PS2DEV" qmake "$REPO_ROOT/RockDroid.pro" CONFIG+=playstation2
make -j"$NCPU"

echo "Built $REPO_ROOT/build/rockbot.elf"
