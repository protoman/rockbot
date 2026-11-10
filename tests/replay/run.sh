#!/usr/bin/env bash
# Run the game headless with a replay script and write the per-frame trace (see aux_tools/replay.h).
# Usage: tests/replay/run.sh <rockbot binary> <script.replay> <trace output> [game folder name, default RockDroid1] [stage, default 0]
# The binary is copied to a temp folder (games/fonts/shared are linked) and HOME points to an empty temp folder,
# so neither your saves nor your config are touched.
set -euo pipefail
BIN="$(readlink -f "${1:?binary}")"; SCRIPT="$(readlink -f "${2:?script}")"; TRACE="$(readlink -m "${3:?trace}")"
GAME="${4:-RockDroid1}"; STAGE="${5:-0}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/home" "$WORK/run"
cp "$BIN" "$WORK/run/rockbot_linux"
for d in games fonts shared; do ln -s "$ROOT/build/$d" "$WORK/run/$d"; done
cd "$WORK/run"
ROCKBOT_REPLAY="$SCRIPT" ROCKBOT_TRACE="$TRACE" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy HOME="$WORK/home" \
    timeout -s KILL "${REPLAY_TIMEOUT:-600}" ./rockbot_linux --quickload --stage "$STAGE" --gamename "$GAME" > "$TRACE.log" 2>&1 || true
tail -1 "$TRACE" 2>/dev/null
