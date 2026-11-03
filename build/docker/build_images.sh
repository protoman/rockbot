#!/usr/bin/env bash
# Build the SDK images (rockbot-psp, -ps2, -ps3, -wii, -windows, -dingux, -pocketgo).
# Usage: build/docker/build_images.sh [target...]   (default: all)
# SDK_DIR (default /mnt/ExtraData/Devel/SDK) is the source of the SDKs baked into the images.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SDK_DIR="${SDK_DIR:-/mnt/ExtraData/Devel/SDK}"
TARGETS=("$@")
[ ${#TARGETS[@]} -gt 0 ] || TARGETS=(psp ps2 ps3 wii windows dingux pocketgo android)
for t in "${TARGETS[@]}"; do
    echo "=== rockbot-$t ==="
    if [ "$t" = android ]; then
        docker buildx build --load -f "$ROOT/ports/android/Dockerfile" -t rockbot-android "$ROOT/ports/android"
        continue
    fi
    docker buildx build --load -f "$ROOT/ports/docker/Dockerfile" --target "$t" \
        --build-context "sdk=$SDK_DIR" -t "rockbot-$t" "$ROOT/ports/docker"
done
