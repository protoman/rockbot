#!/usr/bin/env bash
# Debug the PSP port on a real PSP over USB with psplink (crash dump with registers and PC).
#
# On the PSP: start psplink (from the memory stick) and connect the USB cable.
#
#   ports/psp/psplink.sh stage      builds nothing; copies the last build into the folder served to the PSP
#   ports/psp/psplink.sh host       serves that folder as host0: (run it, leave it running)
#   ports/psp/psplink.sh shell      opens pspsh (in another terminal) - then type:  ldstart host0:/rockbot.prx
#   ports/psp/psplink.sh addr <pc>  maps a crash address to a function. Give the module load address too:
#                                   ports/psp/psplink.sh addr 0x08826598 0x08804000
#                                   (load address: `modinfo` in pspsh, or "module text=" in rockbot_boot.log)
#
# usbhostfs_pc needs access to the USB device: run it with sudo once, or add a udev rule:
#   SUBSYSTEM=="usb", ATTR{idVendor}=="054c", ATTR{idProduct}=="01c9", MODE="0666"   (/etc/udev/rules.d/99-psp.rules)
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PSPDEV="${PSPDEV:-/mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev}"
export PATH="$PSPDEV/bin:$PATH"
SERVE="$ROOT/build-psp/psplink_host"

case "${1:-}" in
    stage)
        [ -f "$ROOT/build-psp/rockbot_psp.prx" ] || { echo "build first: ports/psp/build.sh" >&2; exit 1; }
        rm -rf "$SERVE"; mkdir -p "$SERVE"
        cp -a "$ROOT/build/packages/psp_dist/Rockbot1/." "$SERVE/"
        cp "$ROOT/build-psp/rockbot_psp.prx" "$SERVE/rockbot.prx"
        echo "staged in $SERVE (rockbot.prx + games/ fonts/ shared/)" ;;
    host)
        [ -d "$SERVE" ] || "$0" stage
        echo "serving $SERVE as host0: (Ctrl+C to stop)"
        exec usbhostfs_pc -c "$SERVE" ;;   # -c: case-insensitive names, like the FAT memory stick
    shell)
        exec pspsh ;;
    addr)
        pc="${2:?usage: $0 addr <pc> [load-address]}"; base="${3:-0x08804000}"
        off=$(printf '0x%x' $(( pc - base )))
        echo "offset $off in rockbot:"
        psp-addr2line -f -C -e "$ROOT/build-psp/rockbot_psp" "$off" ;;
    *) sed -n 2,16p "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
esac
