#!/usr/bin/env bash
# Linux x86_64 .deb (Debian/Ubuntu): builds natively (CMake, SDL2) and creates
# Rockbot<N>_Linux_x64_<version>.deb, installing into /opt/rockbot<N> with a /usr/bin/rockbot<N>
# launcher and a menu entry.
# Usage: ./deb.sh [1|2]     (asks which game if omitted)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

NAME="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_Linux_x64_$VERSIONNAME.deb"
ROOT="$PWD/deb/$NAME"

../../build-linux.sh sdl2

rm -rf "$ROOT"
mkdir -p "$ROOT/DEBIAN" "$ROOT/opt/$NAME" "$ROOT/usr/bin" "$ROOT/usr/share/applications" "$ROOT/usr/share/pixmaps"
cp ../rockbot_linux "$ROOT/opt/$NAME/rockbot_linux"
stage_game_data "$ROOT/opt/$NAME"
cp "$PWD/../games/Rockbot$version_number/images/icon_android.png" "$ROOT/usr/share/pixmaps/$NAME.png" 2>/dev/null \
    || cp "$PWD/../games/Rockbot$version_number/images/icon_32px.png" "$ROOT/usr/share/pixmaps/$NAME.png"

cat > "$ROOT/usr/bin/$NAME" <<LAUNCHER
#!/bin/sh
cd /opt/$NAME && exec ./rockbot_linux "\$@"
LAUNCHER
cat > "$ROOT/usr/share/applications/$NAME.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Rockbot $version_number
Comment=Classic action-platformer engine and game
Exec=$NAME
Icon=$NAME
Terminal=false
Categories=Game;ActionGame;
DESKTOP
chmod 755 "$ROOT/usr/bin/$NAME" "$ROOT/opt/$NAME/rockbot_linux"

chmod -R u=rwX,go=rX "$ROOT"
INSTALLED_SIZE="$(du -sk "$ROOT" | cut -f1)"
cat > "$ROOT/DEBIAN/control" <<CONTROL
Package: $NAME
Version: $VERSIONNAME
Section: games
Priority: optional
Architecture: amd64
Depends: libc6, libstdc++6, libsdl2-2.0-0, libsdl2-image-2.0-0, libsdl2-mixer-2.0-0, libsdl2-ttf-2.0-0, libsdl2-gfx-1.0-0, libx11-6
Installed-Size: $INSTALLED_SIZE
Maintainer: Iuri Fiedoruk <protoman@upperland.net>
Homepage: http://rockbot.upperland.net
Description: Rockbot $version_number - classic action-platformer
 Rockbot is a game engine and game in the style of the classic Mega Man
 series, built with SDL2.
CONTROL

rm -f "$OUT"
fakeroot dpkg-deb --build "$ROOT" "$OUT" >/dev/null
dpkg-deb -I "$OUT" | grep -E "Package|Version|Architecture|Installed-Size"
echo "Package: $OUT"
