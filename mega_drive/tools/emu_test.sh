#!/bin/bash
# Automated BlastEm test harness for the RockDroid Mega Drive ROM.
#
# Drives the built-in debugger through a FIFO: advances frame counts, injects
# START via XTEST, screenshots the emulator window at the intro, title and
# stage, then quits. Requires the flatpak override granting write access to
# /tmp/opencode (BlastEm is sandboxed with home:ro).
#
# Usage:  tools/emu_test.sh          (after make)
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ROM="$ROOT/out/rom.bin"
WORK=/tmp/opencode/md-debug
LOG="$WORK/debug.log"
LIVE="$WORK/script.err"
PIPE="$WORK/cmd.pipe"

mkdir -p "$WORK"
[ -f "$ROM" ] || { echo "ROM not found: $ROM (run make first)" >&2; exit 1; }

# A leftover instance holds the X display and blocks the new one.
for pid in $(pgrep -f 'blastem\.bin' 2>/dev/null); do kill -9 "$pid" 2>/dev/null; done
sleep 0.5

rm -f "$LOG" "$LIVE" "$WORK"/shot_*.png "$PIPE"
mkfifo "$PIPE"

# FIFO on stdin keeps the debugger prompt alive; script(1) provides the pty.
# Sleep before opening the write end: both sides of a FIFO block until the
# other end opens, but script must reach its open() first or the handshake
# deadlocks under some shells.
script -qec "timeout 120 flatpak run com.retrodev.blastem -d '$ROM'" "$LOG" \
    < "$PIPE" >"$LIVE" 2>&1 &
EMU_PID=$!
sleep 1
exec 3>"$PIPE"

send() { printf '%s\n' "$1" >&3; }

# Wait until the live output contains a pattern. script(1) buffers its
# session LOG file until exit, so poll its stdout/stderr capture instead.
wait_log() {
    local pat=$1 t=${2:-20}
    for _ in $(seq 1 $((t * 4))); do
        grep -aq "$pat" "$LIVE" 2>/dev/null && return 0
        sleep 0.25
    done
    echo "timeout waiting for '$pat' in $LIVE" >&2
    return 1
}

# The debugger reprints its prompt after finishing a command; count prompts
# to know when a 'frames N' has completed.
prompt_count() { grep -ac '^> ' "$LIVE" 2>/dev/null || echo 0; }

# Send a command and wait for the debugger to finish it (next prompt).
send_wait() {
    local before
    before=$(prompt_count)
    send "$1"
    for _ in $(seq 1 120); do
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

# Debugger stops at the first instruction of the boot ROM.
wait_log 'move #\$2700' 15 || { exec 3>&-; kill $EMU_PID 2>/dev/null; exit 1; }

WID=$(xdotool search --name "BlastEm" 2>/dev/null | head -1)
if [ -z "${WID:-}" ]; then
    echo "BlastEm window not found" >&2
    exec 3>&-; kill $EMU_PID 2>/dev/null; exit 1
fi
xdotool windowactivate --sync "$WID"

# Intro logos: ~230 frames total; catch it mid-intro.
send_wait 'frames 120'
shot intro

# Finish the intro; title screen waits for START.
send_wait 'frames 150'
shot title

# Press START. XTEST only (no --window): --window sends synthetic XSendEvent
# which SDL ignores; bare xdotool key uses XTEST which is indistinguishable
# from real hardware input. SDL only pumps events DURING emulation, so the
# keydown must be followed by at least one resumed frame before the title
# loop can see it. Pulse across several small chunks for reliability.
xdotool windowactivate --sync "$WID"
xdotool keydown Return
send_wait 'frames 2'   # SDL pumps: sees keydown, queues it
send_wait 'frames 5'   # title loop samples pad -> START -> breaks
xdotool keyup Return
send_wait 'frames 2'

# Stage loaded; advance a bit and capture.
send_wait 'frames 60'
shot stage

# Walk right for ~3 seconds (180 frames) and capture where we end up.
xdotool keydown Right
send_wait 'frames 180'
xdotool keyup Right
shot stage_walked

send_wait 'quit'
exec 3>&-
wait $EMU_PID 2>/dev/null

echo "done"
