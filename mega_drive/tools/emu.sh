#!/usr/bin/env bash
#
# Reusable BlastEm controller for the Mega Drive port.
#
# Why this exists: ad-hoc `pkill -f blastem` matches the shell that runs it
# (the command line contains "com.retrodev.blastem") and kills your own
# prompt; and a launched emulator that is not detected/closed lingers. This
# script is the one place that knows how to start, detect, control and stop
# the emulator safely.
#
# Usage (source it in a test script, or run it directly):
#   source tools/emu.sh
#     emu_start [rom]          start (kills any previous instance first)
#     emu_running             -> exit 0 if up
#     emu_window              -> print the X11 window id
#     emu_key KEY [HOLD_MS]   focus + tap a key (e.g. Right, Return, x)
#     emu_shot NAME           screenshot the window to $EMU_WORK/NAME.png
#     emu_stop                terminate cleanly (flatpak kill + exact-name kill)
#   tools/emu.sh start|stop|status|window|shot NAME|key KEY [ms]
#
# Safety rules baked in:
#   * the emulator runs as comm `blastem.bin`; we kill by that EXACT name
#     (`pgrep -x`), never `pkill -f`.
#   * `flatpak kill com.retrodev.blastem` is the primary, proper shutdown.
#   * `emu_start` launches under `timeout` so a forgotten emulator cannot
#     linger forever (override with EMU_TIMEOUT seconds; 0 disables).
#   * the shell that calls stop/start is never a target.

set -u

EMU_APP=com.retrodev.blastem
EMU_PROJ=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
EMU_ROM_DEFAULT="$EMU_PROJ/out/rom.bin"
EMU_WORK=${EMU_WORK:-/tmp/opencode/md-emu}
EMU_TIMEOUT=${EMU_TIMEOUT:-600}
EMU_STATE="$EMU_WORK/emu.state"

# ---------------------------------------------------------------- detection
emu_bin_pids() { pgrep -x blastem.bin 2>/dev/null; }

emu_running() { [ -n "$(emu_bin_pids)" ]; }

# Window belonging to the CURRENT instance: prefer the wid we recorded at
# launch, and only accept it while it is still a live window. This avoids
# grabbing a stale window from a previously killed instance (xdotool window
# ids are recycled and `search | head -1` can return the wrong one).
emu_window() {
    local wid=""
    [ -f "$EMU_STATE" ] && wid=$(sed -n 's/^wid=//p' "$EMU_STATE")
    if [ -n "$wid" ] && timeout 3 xdotool getwindowgeometry "$wid" >/dev/null 2>&1; then
        echo "$wid"; return 0
    fi
    timeout 3 xdotool search --name 'BlastEm' 2>/dev/null | tail -1
}

# windows that do NOT belong to the current instance
emu_other_windows() {
    local w keep
    keep=$([ -f "$EMU_STATE" ] && sed -n 's/^wid=//p' "$EMU_STATE")
    for w in $(xdotool search --name 'BlastEm' 2>/dev/null); do
        [ "$w" = "$keep" ] && continue
        echo "$w"
    done
}

# Graceful quit through BlastEm's Esc menu (Resume, Load ROM, Lock On,
# Save State, Load State, Settings, Exit). This matters for SRAM: BlastEm
# only writes save.sram back on a normal exit; flatpak kill/SIGKILL skips it.
emu_quit_menu() {
    local wid i
    wid=$(emu_window) || return 1
    [ -z "$wid" ] && return 1
    timeout 3 xdotool windowactivate "$wid" 2>/dev/null
    timeout 3 xdotool windowraise "$wid" 2>/dev/null
    timeout 3 xdotool key Escape 2>/dev/null
    sleep 0.4
    for i in 1 2 3 4 5 6; do
        timeout 3 xdotool key Down 2>/dev/null
        sleep 0.08
    done
    timeout 3 xdotool key Return 2>/dev/null
    for i in $(seq 1 24); do emu_running || return 0; sleep 0.25; done
    return 1
}

# ---------------------------------------------------------------- control
emu_stop() {
    # 1) normal exit first (flushes SRAM)
    emu_quit_menu || true
    # 2) proper shutdown of the flatpak instance
    timeout 10 flatpak kill "$EMU_APP" >/dev/null 2>&1 || true
    # 3) exact-name kill for anything left (e.g. launched outside flatpak)
    local pids
    pids=$(emu_bin_pids)
    [ -n "$pids" ] && kill $pids 2>/dev/null
    local i
    for i in $(seq 1 40); do emu_running || break; sleep 0.25; done
    pids=$(emu_bin_pids)
    [ -n "$pids" ] && kill -9 $pids 2>/dev/null
    rm -f "$EMU_STATE"
    return 0
}

emu_start() {
    local rom=${1:-$EMU_ROM_DEFAULT}
    if [ ! -f "$rom" ]; then echo "emu: ROM not found: $rom" >&2; return 1; fi
    emu_stop
    mkdir -p "$EMU_WORK"

    # windows that exist BEFORE launch -- the emulator's window is a new one
    local pre_wins; pre_wins=$(xdotool search --name 'BlastEm' 2>/dev/null | tr '\n' ' ')

    if [ "${EMU_TIMEOUT:-0}" -gt 0 ] 2>/dev/null; then
        setsid timeout -k 5 "$EMU_TIMEOUT" flatpak run "$EMU_APP" "$rom" \
            >"$EMU_WORK/emu.out" 2>"$EMU_WORK/emu.err" </dev/null &
    else
        setsid flatpak run "$EMU_APP" "$rom" \
            >"$EMU_WORK/emu.out" 2>"$EMU_WORK/emu.err" </dev/null &
    fi

    # detect the actual emulator process, not just the launcher
    local i
    for i in $(seq 1 80); do emu_running && break; sleep 0.25; done
    if ! emu_running; then
        echo "emu: failed to start ($rom); see $EMU_WORK/emu.err" >&2
        return 1
    fi

    # the window must NOT be one of the pre-existing ones
    local wid=""
    for i in $(seq 1 80); do
        local w
        for w in $(xdotool search --name 'BlastEm' 2>/dev/null); do
            case " $pre_wins " in *" $w "*) ;; *) wid=$w; break;; esac
        done
        [ -n "$wid" ] && break
        sleep 0.25
    done

    printf 'pids=%s\nwid=%s\nrom=%s\ntimeout=%s\n' \
        "$(emu_bin_pids | tr '\n' ',')" "$wid" "$rom" "$EMU_TIMEOUT" >"$EMU_STATE"
    echo "emu: started (pids=$(emu_bin_pids | tr '\n' ',') wid=${wid:-none} timeout=${EMU_TIMEOUT}s)"
    [ -n "$wid" ] || { echo "emu: window not found (still running?)" >&2; return 1; }
    return 0
}

emu_focus() {
    local wid; wid=$(emu_window); [ -z "$wid" ] && return 1
    timeout 3 xdotool windowactivate "$wid" 2>/dev/null
    timeout 3 xdotool windowraise "$wid" 2>/dev/null
}

# Focus + tap a key, holding it HOLD_MS so the ROM polls it at 60fps.
# Keys are sent globally (SDL ignores XSendEvent); focus is re-asserted first.
emu_key() {
    local key=$1 hold=${2:-80}
    emu_focus || { echo "emu: no window" >&2; return 1; }
    timeout 3 xdotool keydown "$key" 2>/dev/null
    sleep "$(awk "BEGIN{print $hold/1000}")"
    timeout 3 xdotool keyup "$key" 2>/dev/null
}

emu_shot() {
    local name=${1:-shot} wid
    wid=$(emu_window) || { echo "emu: no window" >&2; return 1; }
    [ -z "$wid" ] && { echo "emu: no window" >&2; return 1; }
    sleep 0.3
    timeout 8 import -window "$wid" "$EMU_WORK/$name.png" 2>/dev/null && echo "$EMU_WORK/$name.png"
}

emu_status() {
    if emu_running; then
        echo "emu: running (pids=$(emu_bin_pids | tr '\n' ',') wid=$(emu_window))"
        return 0
    fi
    echo "emu: not running"
    return 1
}

# ---------------------------------------------------------------- dispatch
if [ "${BASH_SOURCE[0]}" = "$0" ]; then
    case "${1:-}" in
        start)  shift; emu_start "$@" ;;
        stop)   emu_stop ;;
        status) emu_status ;;
        window) emu_window ;;
        shot)   shift; emu_shot "$@" ;;
        key)    shift; emu_key "$@" ;;
        *) echo "usage: $0 {start [rom]|stop|status|window|shot NAME|key KEY [ms]}" >&2; exit 2 ;;
    esac
fi
