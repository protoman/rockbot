#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Cross-compile Rockbot for the Raspberry Pi (Raspberry Pi OS / Debian bookworm, SDL2) inside a
# Docker container, so nothing has to be installed on the host.
#
# Usage:
#   ./build-raspberry.sh              → armhf (32-bit Raspberry Pi OS)
#   ./build-raspberry.sh arm64        → 64-bit Raspberry Pi OS
#   ./build-raspberry.sh armhf --rebuild-image
#
# Output: build/rockbot_raspberry_<arch>     (needs libsdl2, -image, -mixer, -ttf and -gfx on the Pi)
#
# Optional env: ROCKBOT_RPI_IMAGE (default rockbot-rpi:latest), NCPU

ARCH="armhf"
REBUILD=false
for arg in "$@"; do
    case "$arg" in
        armhf|arm64) ARCH="$arg" ;;
        --rebuild-image) REBUILD=true ;;
        -h|--help) sed -n '4,15p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Usage: $0 [armhf|arm64] [--rebuild-image]" >&2; exit 1 ;;
    esac
done

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
IMAGE="${ROCKBOT_RPI_IMAGE:-rockbot-rpi:latest}"
case "$ARCH" in arm64) RPI_TRIPLET="aarch64-linux-gnu" ;; *) RPI_TRIPLET="arm-linux-gnueabihf" ;; esac
BUILD_SUBDIR="res/rpi-${ARCH}"

command -v docker >/dev/null 2>&1 || { echo "Error: docker is required" >&2; exit 1; }

PLATFORM_ARGS=()
case "$(uname -m)" in
    arm64|aarch64) PLATFORM_ARGS=(--platform linux/amd64) ;;
esac

if $REBUILD || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "🛠️ Building the Raspberry Pi cross-build image $IMAGE (first time only)..."
    docker build "${PLATFORM_ARGS[@]}" -t "$IMAGE" -f "$REPO_ROOT/ports/raspberry/Dockerfile" "$REPO_ROOT/ports/raspberry"
fi

echo "🛠️ Cross-compiling for $ARCH in the container..."
docker run --rm "${PLATFORM_ARGS[@]}" \
    -u "$(id -u):$(id -g)" \
    -e "ARCH=$ARCH" -e "RPI_ARCH=$ARCH" -e "RPI_TRIPLET=$RPI_TRIPLET" -e "BUILD_SUBDIR=$BUILD_SUBDIR" -e "NCPU=${NCPU:-$(nproc)}" -e "HOME=/tmp" \
    -v "$REPO_ROOT:/rockbot" -w /rockbot "$IMAGE" bash -euo pipefail -c '
        cmake -S . -B "$BUILD_SUBDIR" \
            -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-rpi.cmake -DRPI_ARCH="$ARCH" \
            -DCMAKE_BUILD_TYPE=Release -DUSE_SDL_VERSION=2
        rm -f build/rockbot_linux   # another platform'\''s binary would otherwise be mistaken for this build'\''s output
        cmake --build "$BUILD_SUBDIR" -j"$NCPU"
        case "$ARCH" in arm64) STRIP=aarch64-linux-gnu-strip ;; *) STRIP=arm-linux-gnueabihf-strip ;; esac
        $STRIP --strip-unneeded build/rockbot_linux
        mv -f build/rockbot_linux "build/rockbot_raspberry_$ARCH"
    '
echo "Built $REPO_ROOT/build/rockbot_raspberry_$ARCH"
