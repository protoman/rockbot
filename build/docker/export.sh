#!/usr/bin/env bash
# Save the Rockbot build images as compressed tar files (backup / move to another machine).
# Usage: build/docker/export.sh [image...]   (default: every rockbot-* image)
# Output: $DOCKER_EXPORT_DIR (default /mnt/ExtraData/Devel/SDK/Docker/export)/<image>.tar.zst
set -euo pipefail
OUT="${DOCKER_EXPORT_DIR:-/mnt/ExtraData/Devel/SDK/Docker/export}"
mkdir -p "$OUT"
IMAGES=("$@")
[ ${#IMAGES[@]} -gt 0 ] || mapfile -t IMAGES < <(docker images --format '{{.Repository}}' | grep '^rockbot-' | sort -u)
for img in "${IMAGES[@]}"; do
    echo "exporting $img ..."
    docker save "$img" | zstd -T0 -q -f -o "$OUT/$img.tar.zst"
done
ls -lh "$OUT"
