#!/usr/bin/env bash
# The frog coil must give the big jump: after landing on the coil the player rises about 110 px (a regular jump rises about 55).
# Usage: tests/replay/coil_check.sh <binary> [game]   (prints OK / FAIL)
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="${1:?binary}"; GAME="${2:-RockDroid1}"
TRACE="$(mktemp)"; trap 'rm -f "$TRACE" "$TRACE.log"' EXIT
ROCKBOT_REPLAY_WEAPON=9 "$HERE/run.sh" "$BIN" "$HERE/coil.replay" "$TRACE" "$GAME" 0 > /dev/null
python3 - "$TRACE" <<'PY'
import re, sys
rows = []
for line in open(sys.argv[1]):
    m = re.search(r'F (\d+).*PLAYER_\d pos=([\-\d.]+),([\-\d.]+) anim=\d+/\d+ dir=\d hp=\d+ jump=(\d)', line)
    if m and int(m.group(1)) >= 10000:
        rows.append((float(m.group(3)), int(m.group(4))))
land = [i for i, (y, j) in enumerate(rows) if 133 <= y <= 137 and j == 0]
if not land:
    print("FAIL: the player never landed on the coil"); sys.exit(1)
k = land[0]
while k < len(rows) and rows[k][0] >= 134: k += 1
apex = 999
while k < len(rows) and rows[k][0] < 134:
    apex = min(apex, rows[k][0]); k += 1
rise = 136 - apex
print(("OK" if rise > 90 else "FAIL") + ": the jump from the coil rises %d px" % rise)
sys.exit(0 if rise > 90 else 1)
PY
