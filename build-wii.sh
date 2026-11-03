#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Cross-compile Rockbot for the Nintendo Wii with the current devkitPro devkitPPC + libogc and the
# Wii SDL 1.2 ports, inside the official devkitpro/devkitppc Docker image (nothing to install).
#
# Usage:
#   ./build-wii.sh
#
# Output: build-wii/boot.dol
#
# Optional env: ROCKBOT_WII_IMAGE (default devkitpro/devkitppc:latest), NCPU

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
IMAGE="${ROCKBOT_WII_IMAGE:-devkitpro/devkitppc:latest}"

case "${1:-}" in
    "") ;;
    -h|--help) sed -n '4,13p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "Usage: $0" >&2; exit 1 ;;
esac

command -v docker >/dev/null 2>&1 || { echo "Error: docker is required" >&2; exit 1; }

echo "🛠️ Cross-compiling for the Wii in $IMAGE..."
docker run --rm \
    -u "$(id -u):$(id -g)" \
    -e "NCPU=${NCPU:-$(nproc)}" -e "HOME=/tmp" \
    -v "$REPO_ROOT:/rockbot" -w /rockbot "$IMAGE" bash -euo pipefail -c '
        export DEVKITPRO=/opt/devkitpro DEVKITPPC=/opt/devkitpro/devkitPPC
        export PATH="$DEVKITPPC/bin:$DEVKITPRO/tools/bin:$PATH"
        cmake -S . -B build-wii \
            -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Wii.cmake \
            -DCMAKE_BUILD_TYPE=Release
        cmake --build build-wii -j"$NCPU"
    '
echo "Built $REPO_ROOT/build-wii/boot.dol"
