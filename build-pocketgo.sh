#!/bin/bash
set -e
set -o pipefail

# PocketGo (arm musl, SDL 1.2, static libs) build with the Buildroot SDK.
# Usage: ./build-pocketgo.sh
# Optional env:
#   POCKETGO_SDK  Buildroot host dir (default: /mnt/ExtraData/Devel/SDK/PocketGo/output/host)
#   NCPU          make parallelism (default: nproc)

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SDK="${POCKETGO_SDK:-/mnt/ExtraData/Devel/SDK/PocketGo/output/host}"
BUILD_DIR="$REPO_ROOT/build-pocketgo"

if [ ! -f "$SDK/share/buildroot/toolchainfile.cmake" ]; then
    echo "PocketGo SDK not found at $SDK (set POCKETGO_SDK)" >&2
    exit 1
fi

cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$SDK/share/buildroot/toolchainfile.cmake" \
    -DROCKBOT_HANDHELD=pocketgo
cmake --build "$BUILD_DIR" -j"${NCPU:-$(nproc)}"

echo "Built $REPO_ROOT/build/rockbot_pocketgo"
