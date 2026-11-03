#!/usr/bin/env bash
# ONE launch: start the emulator, take a window shot every ~0.7 s into
# $EMU_WORK/gl_NNNN.png (a frame is kept only when it differs from the previous
# one, capped at 400), until the file $EMU_WORK/gl.stop exists; then close the
# emulator through its own menu (flushes SRAM). Never relaunches.
set -u
cd "$(dirname "$0")/.."
source tools/emu.sh
mkdir -p "$EMU_WORK"; rm -f "$EMU_WORK"/gl_*.png "$EMU_WORK/gl.stop"
emu_running && { echo "an emulator is already running - not launching" >&2; exit 3; }
emu_start "${1:-out/rom.bin}" >/dev/null 2>&1
sleep 3
n=0; last=""
while [ ! -e "$EMU_WORK/gl.stop" ] && [ $n -lt 400 ]; do
    if emu_shot gl_tmp >/dev/null 2>&1; then
        sum=$(md5sum "$EMU_WORK/gl_tmp.png" | cut -d' ' -f1)
        if [ "$sum" != "$last" ]; then
            n=$((n+1)); mv "$EMU_WORK/gl_tmp.png" "$(printf '%s/gl_%04d.png' "$EMU_WORK" $n)"; last=$sum
        fi
    fi
    sleep 0.4
done
emu_stop >/dev/null 2>&1
echo "frames kept: $n"
