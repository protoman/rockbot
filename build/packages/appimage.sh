#!/usr/bin/env bash
# Linux x86_64 AppImage: builds natively (CMake, SDL2), assembles an AppDir with the game, its
# data and the SDL libraries it needs, and packs it with appimagetool into
# Rockbot<N>-<version>-x86_64.AppImage + .zsync (update information for AppImageUpdate)   (no 'Linux' in the name: appimage.github.io rejects it)
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
OUT="$PWD/Rockbot${version_number}-$VERSIONNAME-x86_64.AppImage"
APPDIR="$PWD/appimage/Rockbot$version_number.AppDir"

../../build-linux.sh sdl2

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/$NAME"
cp ../rockbot_linux "$APPDIR/usr/bin/rockbot_linux"
stage_game_data "$APPDIR/usr/share/$NAME"

# --- bundle the shared libraries the game needs, except the ones every system provides ---
# Only glibc, the graphics stack (GL/EGL/Vulkan/DRM/GBM, X11/xcb, Wayland, xkbcommon), udev and fontconfig
# come from the host. Everything else is bundled, the audio libraries too: SDL2_mixer links libfluidsynth,
# which needs libpipewire, libjack, libpulse, libasound and libdbus at load time, and a minimal system
# (the appimage.github.io test machine) has none of them ("error while loading shared libraries").
is_system_lib() {
    case "$(basename "$1")" in
        ld-linux*|libc.so*|libm.so*|libdl.so*|libpthread.so*|librt.so*|libutil.so*|libresolv.so*|libnsl.so*|libcrypt.so*|\
        libgcc_s.so*|libstdc++.so*|libGL.so*|libGLX*|libGLdispatch*|libEGL*|libOpenGL*|libGLESv*|libdrm*|libgbm*|libvulkan*|\
        libX*|libxcb*|libwayland*|libxkbcommon*|libudev*|libfontconfig*|\
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

# Every library the game and the bundled libraries need must be bundled or be one of the host ones above.
missing=""
for f in "$APPDIR/usr/bin/rockbot_linux" "$APPDIR"/usr/lib/*.so*; do
    for need in $(readelf -d "$f" 2>/dev/null | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p'); do
        [ -e "$APPDIR/usr/lib/$need" ] && continue
        is_system_lib "$need" && continue
        missing="$missing $need"
    done
done
if [ -n "$missing" ]; then
    echo "Error: libraries needed but neither bundled nor provided by the host:$missing" >&2
    exit 1
fi

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
APP_ID="io.github.protoman.$NAME"
cat > "$APPDIR/$APP_ID.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Rockbot $version_number
Comment=Classic action-platformer engine and game
Exec=rockbot_linux
Icon=$NAME
Terminal=false
Categories=Game;ActionGame;
DESKTOP

# AppStream metadata (shown by software catalogs such as appimage.github.io)
mkdir -p "$APPDIR/usr/share/metainfo"
cat > "$APPDIR/usr/share/metainfo/$APP_ID.appdata.xml" <<APPSTREAM
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>$APP_ID</id>
  <name>Rockbot $version_number</name>
  <summary>Classic NES-style action-platformer in the spirit of Mega Man</summary>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-2.0-only</project_license>
  <developer id="io.github.protoman">
    <name>Iuri Fiedoruk</name>
  </developer>
  <description>
    <p>Rockbot is an open-source 2D action-platformer engine in the spirit of the classic NES Mega Man games, and this is Rockbot $version_number, one of the two games that come with it.</p>
    <p>Run, jump, slide and use the charged shot through eight stages, beat the bosses to get their weapons, and collect E, W and S tanks and armor pieces. The game has cutscenes, an intro and an ending, several difficulty levels, and keyboard and gamepad support.</p>
    <p>All the content (stages, enemies, weapons, scenes, graphics) is data made with the Rockbot editor, so anyone can create and distribute a game for the engine.</p>
  </description>
  <launchable type="desktop-id">$APP_ID.desktop</launchable>
  <url type="homepage">https://github.com/protoman/rockbot</url>
  <url type="bugtracker">https://github.com/protoman/rockbot/issues</url>
  <screenshots>
    <screenshot type="default">
      <image>https://raw.githubusercontent.com/protoman/rockbot/master/docs/screenshot-game.png</image>
    </screenshot>
  </screenshots>
  <categories>
    <category>Game</category>
    <category>ActionGame</category>
  </categories>
  <content_rating type="oars-1.1">
    <content_attribute id="violence-cartoon">mild</content_attribute>
  </content_rating>
  <provides>
    <binary>rockbot_linux</binary>
  </provides>
  <releases>
    <release version="$VERSIONNAME" date="$(date +%F)"/>
  </releases>
</component>
APPSTREAM
chmod 644 "$APPDIR/usr/share/metainfo/$APP_ID.appdata.xml"

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
# Update information (AppImageUpdate): the newest GitHub release's file of the same game. appimagetool also
# writes <AppImage>.zsync next to the AppImage (needs zsyncmake): upload it to the release together with it.
UPDATE_INFO="gh-releases-zsync|protoman|rockbot|latest|Rockbot${version_number}-*-x86_64.AppImage.zsync"
rm -f "$OUT.zsync"
ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 "$TOOL" -u "$UPDATE_INFO" "$APPDIR" "$OUT"
chmod +x "$OUT"
[ -f "$OUT.zsync" ] || echo "Warning: $OUT.zsync was not created (install zsync)" >&2
echo "Package: $OUT"
