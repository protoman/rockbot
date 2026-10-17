#!/bin/bash
# Object test-room harness: pill x16, 1UP x32, checkpoint x48, fly platform
# x112 (left of spawn), fall platform x208 (right of spawn), flat ground 176.
# Exercises side collision, land-activation, fall cycle and fly carry;
# screenshots between steps.
#
# Usage:  tools/emu_platform_test.sh   (after make)
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ROM="$ROOT/out/rom.bin"
WORK=/tmp/opencode/md-debug
LOG="$WORK/debug.log"
LIVE="$WORK/script.err"
PIPE="$WORK/cmd.pipe"

mkdir -p "$WORK"
[ -f "$ROM" ] || { echo "ROM not found: $ROM (run make first)" >&2; exit 1; }

for pid in $(pgrep -f 'blastem\.bin' 2>/dev/null); do kill -9 "$pid" 2>/dev/null; done
sleep 0.5

rm -f "$LOG" "$LIVE" "$WORK"/shot_*.png "$PIPE"
mkfifo "$PIPE"

script -qec "timeout 240 flatpak run com.retrodev.blastem -d '$ROM'" "$LOG" \
    < "$PIPE" >"$LIVE" 2>&1 &
EMU_PID=$!
sleep 1
exec 3>"$PIPE"

send() { printf '%s\n' "$1" >&3; }

wait_log() {
    local pat=$1 t=${2:-20}
    for _ in $(seq 1 $((t * 4))); do
        grep -aq "$pat" "$LIVE" 2>/dev/null && return 0
        sleep 0.25
    done
    echo "timeout waiting for '$pat' in $LIVE" >&2
    return 1
}

prompt_count() { grep -ac '^> ' "$LIVE" 2>/dev/null || echo 0; }

send_wait() {
    local before
    before=$(prompt_count)
    send "$1"
    for _ in $(seq 1 160); do
        [ "$(prompt_count)" -gt "$before" ] && return 0
        sleep 0.25
    done
    echo "timeout waiting for command to finish: $1" >&2
    return 1
}

shot() {
    local name=$1
    sleep 0.3
    local wid
    wid=$(xdotool search --name "BlastEm" 2>/dev/null | head -1)
    if [ -n "$wid" ]; then
        import -window "$wid" "$WORK/shot_$name.png" 2>/dev/null \
            && echo "screenshot: $WORK/shot_$name.png"
    else
        echo "BlastEm window not found" >&2
    fi
}

# Hold a key for N frames (SDL only pumps input during resumed frames).
key_hold() {
    xdotool keydown "$1"
    send_wait "frames $2"
    xdotool keyup "$1"
    send_wait 'frames 2'
}

key_tap() {
    xdotool keydown "$1"
    send_wait 'frames 3'
    xdotool keyup "$1"
    send_wait 'frames 2'
}

wait_log 'move #\$2700' 15 || { exec 3>&-; kill $EMU_PID 2>/dev/null; exit 1; }

WID=$(xdotool search --name "BlastEm" 2>/dev/null | head -1)
if [ -z "${WID:-}" ]; then
    echo "BlastEm window not found" >&2
    exec 3>&-; kill $EMU_PID 2>/dev/null; exit 1
fi
xdotool windowactivate --sync "$WID"

# Intro logos + title; START to enter the stage.
send_wait 'frames 120'
send_wait 'frames 150'
xdotool windowactivate --sync "$WID"
xdotool keydown Return
send_wait 'frames 2'
send_wait 'frames 5'
xdotool keyup Return
send_wait 'frames 2'

send_wait 'frames 60'
shot 10_spawn

# Geometry (game space, ground y160 = row10; col0 = full-height wall):
#   spawn PX144 (box 150-169);
#   fly  block x112-144 y144-160 (LEFT of spawn, flush on floor);
#   fall block x208-224 y128   (RIGHT of spawn, 32px above ground);
#   pill x16; 1UP x32; checkpoint x48.
# Walk speed ~1.7px/frame. Full jump ~56px arc (~42f airtime, C held >=18f).
# Fall-side clamp: box right (x+25) hits fall left 208 -> x183.
# Fly-side clamp:  box left  (x+6)  hits fly right 144 -> x138.

# 1) Walk right -> fall side-block at x183.
key_hold Right 28
shot 20_fall_side        # expect PX0183, A home

# 2) Jump right onto fall top y128 -> activate (13f delay) -> fall+carry+return.
xdotool keydown Right
xdotool keydown C
send_wait 'frames 12'
xdotool keyup C
send_wait 'frames 10'
xdotool keyup Right
send_wait 'frames 15'
shot 30_fall_land
send_wait 'frames 30'
shot 31_fall_falling
send_wait 'frames 130'
shot 32_fall_returned

# 3) Walk left past spawn -> fly side-block at x138.
key_hold Left 80
shot 40_fly_side         # expect PX0138, B home

# 4) Jump left onto fly top y144 -> activate -> rise + CARRY (PY tracks B y).
xdotool keydown Left
xdotool keydown C
send_wait 'frames 18'
xdotool keyup C
send_wait 'frames 14'
xdotool keyup Left
send_wait 'frames 12'
shot 50_fly_land
send_wait 'frames 25'
shot 51_fly_rising       # PY should track B y (CARRY)
send_wait 'frames 55'
shot 52_fly_high
send_wait 'frames 150'
shot 53_fly_done

send_wait 'quit'
exec 3>&-
wait $EMU_PID 2>/dev/null

echo "done"
