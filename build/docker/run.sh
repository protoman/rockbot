#!/usr/bin/env bash
# Run a command inside one of the Rockbot build images, with the repository mounted.
#
# Usage: build/docker/run.sh <platform> [--build] <command...>
#   platform: linux | rpi | psp | ps2 | ps3 | wii | windows | dingux | pocketgo | android   (image rockbot-<platform>; Dockerfile in ports/<dir>/)
#   --build : (re)build the image first
# Example: build/docker/run.sh linux ./build-linux.sh sdl2
#
# Files created in the repository are owned by your user, not root.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PLATFORM="${1:?usage: $0 <platform> [--build] <command...>}"; shift
case "$PLATFORM" in
    linux) DF_DIR=linux ;;
    rpi|raspberry) PLATFORM=rpi; DF_DIR=raspberry ;;
    psp|ps2|ps3|wii|windows|dingux|pocketgo|android) DF_DIR= ;;
    *) echo "unknown platform: $PLATFORM" >&2; exit 1 ;;
esac
IMAGE="rockbot-$PLATFORM"
if [ "${1:-}" = "--build" ] || ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    [ "${1:-}" = "--build" ] && shift
    if [ -n "$DF_DIR" ]; then
        docker build -t "$IMAGE" "$ROOT/ports/$DF_DIR"
    else
        "$ROOT/build/docker/build_images.sh" "$PLATFORM"
    fi
fi
# The Android project builds the game through a link to the repository, created here so the image
# does not depend on where the repository lives.
if [ "$PLATFORM" = android ]; then
    set -- bash -c 'ln -sfn "$0" "$ANDROID_BASE/commandergenius/project/jni/application/rockbot" && exec "$@"' "$ROOT" "$@"
fi
exec docker run --rm -u "$(id -u):$(id -g)" -e HOME=/tmp \
    -e ROCKBOT_IN_DOCKER=1 -v "$ROOT":"$ROOT" -w "$ROOT" "$IMAGE" "$@"
