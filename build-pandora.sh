#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# OpenPandora (Cortex-A8, glibc 2.9, SDL 1.2) build: current GCC built by ports/pandora/build-toolchain.sh
# against the OpenPandora SDK sysroot (the SDK's own GCC 4.3.3 / 4.6.1 are too old for the game's C++11).
# Usage: ./build-pandora.sh
# Optional env:
#   PANDORA_TOOLCHAIN  compiler prefix, holds bin/arm-angstrom-linux-gnueabi-gcc
#                      (default: /mnt/ExtraData/Devel/SDK/Pandora/gcc-16.2.0)
#   PANDORA_SYSROOT    OpenPandora SDK sysroot
#                      (default: /mnt/ExtraData/Devel/SDK/OpenPandora/arm-angstrom-linux-gnueabi)
#   NCPU               make parallelism (default: nproc)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
TOOLCHAIN="${PANDORA_TOOLCHAIN:-/mnt/ExtraData/Devel/SDK/Pandora/gcc-16.2.0}"
SYSROOT="${PANDORA_SYSROOT:-/mnt/ExtraData/Devel/SDK/OpenPandora/arm-angstrom-linux-gnueabi}"
BUILD_DIR="$REPO_ROOT/build-pandora"

if [ ! -x "$TOOLCHAIN/bin/arm-angstrom-linux-gnueabi-gcc" ] || [ ! -d "$SYSROOT/usr/include/SDL" ]; then
    echo "Pandora SDK not found (PANDORA_TOOLCHAIN=$TOOLCHAIN, PANDORA_SYSROOT=$SYSROOT)" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$REPO_ROOT/cmake/toolchain-pandora.cmake" \
    -DPANDORA_TOOLCHAIN="$TOOLCHAIN" -DPANDORA_SYSROOT="$SYSROOT" -DCMAKE_BUILD_TYPE=Release \
    -DROCKBOT_HANDHELD=pandora
cmake --build "$BUILD_DIR" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build/rockbot_pandora"
