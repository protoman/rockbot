#!/bin/bash
# Runs inside the pspdev/pspdev Docker image.
set -e
set -o pipefail

REPO_ROOT="${REPO_ROOT:-/rockbot}"
PACKAGES_DIR="$REPO_ROOT/build/packages"
GAME_DATA_ROOT="$REPO_ROOT/build"
VERSION_NUMBER="${VERSION_NUMBER:-${1:-}}"
NCPU="${NCPU:-$(nproc 2>/dev/null || echo 2)}"

if [[ "$VERSION_NUMBER" != "1" && "$VERSION_NUMBER" != "2" ]]; then
    echo "Usage (container): VERSION_NUMBER=1|2 rockbot-psp-build"
    exit 1
fi

export PSPDEV="${PSPDEV:-/usr/local/pspdev}"
export PATH="${PSPDEV}/bin:${PATH}"

if ! command -v psp-config >/dev/null 2>&1; then
    echo "Error: psp-config not found. Is this the pspdev image?"
    exit 1
fi

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
# Use psp_dist (not PSP/) so macOS case-insensitive FS cannot collide with ports/psp or ./psp.
STAGE_DIR="$PACKAGES_DIR/psp_dist"
GAME_DIR="Rockbot${VERSION_NUMBER}"
OUT_ZIP="$PACKAGES_DIR/RockBot_PSP_${VERSION_NAME}.zip"
BUILD_LOG="$PACKAGES_DIR/psp-build.log"

echo ">> [docker] Rockbot PSP v${VERSION_NUMBER} (${VERSION_NAME})"
echo "📦 PSPDEV=$PSPDEV"
echo "📦 psp-config --pspsdk-path=$(psp-config --pspsdk-path)"

install_psp_packages() {
    if ! command -v psp-pacman >/dev/null 2>&1; then
        echo "⚠️ psp-pacman not available; assuming SDL2 libs are already installed"
        return 0
    fi
    echo "📦 Installing/updating PSP SDL2 packages..."
    psp-pacman -Sy --noconfirm || true
    for pkg in \
        sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx \
        sdl2-image sdl2-mixer sdl2-ttf sdl2-gfx \
        libpng libjpeg-turbo jpeg freetype2 bzip2 libbz2 \
        harfbuzz harfbuzz-icu libvorbis libogg flac mpg123 opusfile \
        libmodplug modplug libxmp mikmod
    do
        if psp-pacman -Q "$pkg" >/dev/null 2>&1; then
            continue
        fi
        if psp-pacman -S --noconfirm --needed "$pkg" >/dev/null 2>&1 \
            || psp-pacman -S --noconfirm "$pkg" >/dev/null 2>&1; then
            echo "  ✓ $pkg"
        fi
    done
}

install_psp_packages

for required in \
    "$GAME_DATA_ROOT/fonts" \
    "$GAME_DATA_ROOT/shared" \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}" \
    "$PACKAGES_DIR/files/psp_icon.png" \
    "$PACKAGES_DIR/files/psp_background.png"
do
    if [[ ! -e "$required" ]]; then
        echo "Error: missing required path: $required"
        exit 1
    fi
done

cd "$REPO_ROOT"
mkdir -p "$PACKAGES_DIR"
: > "$BUILD_LOG"

echo "🛠️ Cleaning previous PSP objects..."
make -f Makefile.psp clean >>"$BUILD_LOG" 2>&1 || true
rm -f EBOOT.PBP PARAM.SFO rockbot_psp.elf rockbot_psp.prx rockbot_psp_strip.elf

echo "🛠️ Building EBOOT.PBP (SDL2 / -j${NCPU})..."
set +e
make -f Makefile.psp -j"$NCPU" \
    VERSION_NUMBER="$VERSION_NUMBER" \
    VERSIONNAME="$VERSION_NAME" \
    2>&1 | tee -a "$BUILD_LOG"
BUILD_STATUS=${PIPESTATUS[0]}
set -e

if [[ "$BUILD_STATUS" -ne 0 || ! -f "$REPO_ROOT/EBOOT.PBP" ]]; then
    echo "Error: PSP build failed (exit $BUILD_STATUS). Last 80 lines:"
    tail -n 80 "$BUILD_LOG" || true
    exit "${BUILD_STATUS:-1}"
fi

echo "📦 Packaging PSP zip..."
rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/$GAME_DIR/games"

cp -f "$REPO_ROOT/EBOOT.PBP" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/fonts" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/shared" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' --exclude='mp3' \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}" \
    "$STAGE_DIR/$GAME_DIR/games/"

if [[ -f "$PACKAGES_DIR/files/psp_icon${VERSION_NUMBER}.png" ]]; then
    cp -f "$PACKAGES_DIR/files/psp_icon${VERSION_NUMBER}.png" "$STAGE_DIR/$GAME_DIR/rockbot.png"
elif [[ -f "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}/images/icon_32px.png" ]]; then
    cp -f "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}/images/icon_32px.png" \
        "$STAGE_DIR/$GAME_DIR/rockbot.png"
fi

rm -f "$STAGE_DIR/$GAME_DIR"/game*.sav \
    "$STAGE_DIR/$GAME_DIR"/config_v*.sav \
    "$STAGE_DIR/$GAME_DIR/data"/game*.sav \
    "$STAGE_DIR/$GAME_DIR/data"/config*.sav 2>/dev/null || true

rm -f "$OUT_ZIP"
(
    cd "$STAGE_DIR"
    zip -qr "$OUT_ZIP" "$GAME_DIR"
)

echo "✅ Build completed successfully"
echo "EBOOT: $REPO_ROOT/EBOOT.PBP"
echo "ZIP:   $OUT_ZIP"
echo "Copy $GAME_DIR/ to ms0:/PSP/GAME/ on the Memory Stick"
