#!/usr/bin/env bash
# Compare two builds of the game on the replay scenarios: any difference in a trace is a change of behaviour.
# Usage: tests/replay/compare.sh <old binary> <new binary>
# (build the "old" one from the commit before the change, e.g. in a git worktree; see tests/replay/README.md)
set -uo pipefail
OLD="${1:?old binary}"; NEW="${2:?new binary}"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="$(mktemp -d)"; trap 'rm -rf "$OUT"' EXIT
# script game stage
SCENARIOS=(
    "intro_rockbot1.replay RockDroid1 0"
    "intro_rockbot1.replay RockDroid1 1"
    "intro_rockbot1.replay RockDroid1 3"
    "intro_rockbot1.replay RockDroid1 5"
    "intro_rockbot1.replay RockDroid2 0"
    "intro_rockbot1.replay RockDroid2 2"
    "mixed.replay RockDroid1 2"
    "mixed.replay RockDroid1 4"
    "mixed.replay RockDroid1 6"
    "mixed.replay RockDroid1 8"
    "mixed.replay RockDroid2 1"
    "mixed.replay RockDroid2 4"
)
failed=0
for sc in "${SCENARIOS[@]}"; do
    set -- $sc
    name="$2_stage$3_${1%.replay}"
    "$HERE/run.sh" "$OLD" "$HERE/$1" "$OUT/$name.old" "$2" "$3" > /dev/null &
    "$HERE/run.sh" "$NEW" "$HERE/$1" "$OUT/$name.new" "$2" "$3" > /dev/null &
    wait
    frames=$(wc -l < "$OUT/$name.old")
    if cmp -s "$OUT/$name.old" "$OUT/$name.new"; then
        echo "same       $name ($frames frames)"
    else
        failed=1
        echo "DIFFERENT  $name: first difference:"
        diff "$OUT/$name.old" "$OUT/$name.new" | head -4 | cut -c1-300
    fi
done
exit $failed
