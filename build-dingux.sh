#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# OpenDingux (GCW0/RG350 class, mipsel uClibc, SDL 1.2) build with the Buildroot SDK.
# Usage: ./build-dingux.sh
# Optional env:
#   OPEN_DINGUX_SDK  toolchain root (default: /mnt/ExtraData/Devel/SDK/opendingux-gcw0-2022-09-22/gcw0-toolchain)
#                    Prebuilt: https://github.com/OpenDingux/buildroot/releases (opendingux-gcw0-toolchain.*.tar.xz)
#   NCPU             make parallelism (default: nproc)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SDK="${OPEN_DINGUX_SDK:-/mnt/ExtraData/Devel/SDK/opendingux-gcw0-2022-09-22/gcw0-toolchain}"
BUILD_DIR="$REPO_ROOT/build-dingux"

if [ ! -f "$SDK/share/buildroot/toolchainfile.cmake" ]; then
    echo "OpenDingux SDK not found at $SDK (set OPEN_DINGUX_SDK)" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$SDK/share/buildroot/toolchainfile.cmake" \
    -DROCKBOT_HANDHELD=dingux
cmake --build "$BUILD_DIR" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build/rockbot.dge"
