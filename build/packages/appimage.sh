#!/usr/bin/env bash
# Linux x86_64 AppImage: builds natively (CMake, SDL2), assembles an AppDir with the game, its
# data and the SDL libraries it needs, and packs it with appimagetool into
# Rockbot<N>_Linux_x64_<version>.AppImage
# Usage: ./appimage.sh [1|2]     (asks which game if omitted)
#
# appimagetool is kept in $LINUX_SDK (default /mnt/ExtraData/Devel/SDK/Linux) and downloaded
# on first use. Set APPIMAGETOOL=/path/to/appimagetool to use your own copy. Without FUSE it is
# run with --appimage-extract-and-run automatically.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

NAME="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_Linux_x64_$VERSIONNAME.AppImage"
APPDIR="$PWD/appimage/Rockbot$version_number.AppDir"

../../build-linux.sh sdl2

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/$NAME"
cp ../rockbot_linux "$APPDIR/usr/bin/rockbot_linux"
stage_game_data "$APPDIR/usr/share/$NAME"

# --- bundle the shared libraries the game needs, except the ones every system provides ---
# (glibc, GL/EGL/X11/Wayland, ALSA/Pulse, fontconfig... must come from the host.)
is_system_lib() {
    case "$(basename "$1")" in
        ld-linux*|libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*|libutil.so*|libresolv.so*|libnsl.so*|libcrypt.so*|\
        libgcc_s.so*|libstdc++.so*|libGL.so*|libGLX*|libGLdispatch*|libEGL*|libOpenGL*|libGLESv*|libdrm*|libgbm*|libvulkan*|\
        libX*|libxcb*|libwayland*|libxkbcommon*|libasound*|libpulse*|libjack*|libpipewire*|libudev*|libdbus*|libsystemd*|\
        libfontconfig*|libfreetype*|libharfbuzz*|libglib-2*|libgobject*|libgio*|libz.so*|libselinux*|libffi*|libbsd*|libmd.so*|\
        linux-vdso*|libnvidia*|libcuda*) return 0 ;;
    esac
    return 1
}
ldd "$APPDIR/usr/bin/rockbot_linux" | awk '/=> \// {print $3}' | while read -r lib; do
    if ! is_system_lib "$lib"; then
        cp -L "$lib" "$APPDIR/usr/lib/"
    fi
done
echo "Bundled libraries: $(ls "$APPDIR/usr/lib" | tr '\n' ' ')"

cat > "$APPDIR/AppRun" <<APPRUN
#!/bin/sh
HERE="\$(dirname "\$(readlink -f "\$0")")"
export LD_LIBRARY_PATH="\$HERE/usr/lib:\${LD_LIBRARY_PATH:-}"
# The game looks for its data in the current directory (read-only is fine: saves go to ~/.rockdroid).
cd "\$HERE/usr/share/$NAME" && exec "\$HERE/usr/bin/rockbot_linux" "\$@"
APPRUN
chmod +x "$APPDIR/AppRun" "$APPDIR/usr/bin/rockbot_linux"

ICON_SRC="$PWD/../games/Rockbot$version_number/images/icon_android.png"
[ -f "$ICON_SRC" ] || ICON_SRC="$PWD/../games/Rockbot$version_number/images/icon_32px.png"
cp "$ICON_SRC" "$APPDIR/$NAME.png"
cp "$ICON_SRC" "$APPDIR/.DirIcon"
cat > "$APPDIR/$NAME.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Rockbot $version_number
Comment=Classic action-platformer engine and game
Exec=rockbot
Icon=$NAME
Terminal=false
Categories=Game;ActionGame;
DESKTOP

# --- pack with appimagetool ---------------------------------------------------------------
LINUX_SDK="${LINUX_SDK:-/mnt/ExtraData/Devel/SDK/Linux}"
TOOL="${APPIMAGETOOL:-$LINUX_SDK/appimagetool-x86_64.AppImage}"
if [ ! -x "$TOOL" ]; then
    mkdir -p "$(dirname "$TOOL")"
    echo "Downloading appimagetool into $(dirname "$TOOL") ..."
    curl -fL --retry 3 -o "$TOOL.partial" \
        https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    mv "$TOOL.partial" "$TOOL"
    chmod +x "$TOOL"
fi
rm -f "$OUT"
ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 "$TOOL" "$APPDIR" "$OUT"
chmod +x "$OUT"
echo "Package: $OUT"
