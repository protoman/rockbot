#!/usr/bin/env bash
# Cross-builds RockBot for Windows from Linux (llvm-mingw, see ../../build-windows.sh) and
# creates Rockbot<N>_Win64_<version>.zip (or _Win32_ for i686): rockbot_windows.exe + the SDL DLLs + fonts/shared/games.
# Usage: ./win32.sh [1|2] [sdl2|sdl3] [x86_64|i686]     (asks which game if omitted)
# The Windows SDK is kept in $WINDOWS_SDK (default /mnt/ExtraData/Devel/SDK/Windows).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"

game="" ; sdl="sdl2" ; arch="x86_64"
for arg in "$@"; do
    case "$arg" in
        1|2) game="$arg" ;;
        sdl2|sdl3) sdl="$arg" ;;
        x86_64|i686) arch="$arg" ;;
        *) echo "Usage: $0 [1|2] [sdl2|sdl3] [x86_64|i686]" >&2; exit 1 ;;
    esac
done
choose_game "$game" || exit 1

# Builds build/rockbot_windows.exe and packages build/packages/Rockbot<N>_Win32_<version>.zip
../../build-windows.sh "$sdl" "$arch" "$version_number"
case "$arch" in i686) tag="Win32" ;; *) tag="Win64" ;; esac
echo "Package: $PWD/Rockbot${version_number}_${tag}_${VERSIONNAME}.zip"
