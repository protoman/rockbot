#!/usr/bin/env bash
# Load the images saved by export.sh (e.g. after reinstalling the machine).
# Usage: build/docker/import.sh [file.tar.zst...]   (default: every file in the export dir)
set -euo pipefail
OUT="${DOCKER_EXPORT_DIR:-/mnt/ExtraData/Devel/SDK/Docker/export}"
FILES=("$@")
[ ${#FILES[@]} -gt 0 ] || FILES=("$OUT"/*.tar.zst)
for f in "${FILES[@]}"; do
    echo "loading $f ..."
    zstd -dc "$f" | docker load
done
