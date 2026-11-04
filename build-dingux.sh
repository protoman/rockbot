#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Dingoo A320 (JZ4740, mips32 soft-float, uClibc 0.9.31, SDL 1.2) build with the legacy OpenDingux 1.2.5
# toolchain (GCC 4.7.1; 32-bit host binaries).
# Usage: ./build-dingux.sh
# Optional env:
#   OPEN_DINGUX_SDK  toolchain root, the folder holding usr/ (default: /mnt/ExtraData/Devel/SDK/opendingux)
#   NCPU             make parallelism (default: nproc)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SDK="${OPEN_DINGUX_SDK:-/mnt/ExtraData/Devel/SDK/opendingux}"
BUILD_DIR="$REPO_ROOT/build-dingux"

if [ ! -f "$SDK/usr/bin/mipsel-linux-gcc" ]; then
    echo "OpenDingux SDK not found at $SDK (set OPEN_DINGUX_SDK)" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$REPO_ROOT/cmake/toolchain-dingux-a320.cmake" \
    -DDINGUX_A320_SDK="$SDK" -DCMAKE_BUILD_TYPE=Release \
    -DROCKBOT_HANDHELD=dingux
cmake --build "$BUILD_DIR" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build/rockbot_dingux.dge"
