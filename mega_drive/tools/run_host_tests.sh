#!/usr/bin/env bash
# Host cost tests: compile the REAL port sources against counting stubs and run
# them natively. No emulator, no SGDK — see tests/host/.
set -eu
cd "$(dirname "$0")/.."
OUT=/tmp/opencode/host-tests
mkdir -p "$OUT"
CC=${CC:-gcc}
CFLAGS="-std=gnu11 -O1 -DMD_HOST_TEST -I tests/host -I src -I res"

echo "== objects cost (work per frame)"
$CC $CFLAGS -o "$OUT/objects_cost" \
    src/object.c tests/host/stubs.c tests/host/objects_cost.c
"$OUT/objects_cost"
