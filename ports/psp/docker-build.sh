#!/bin/bash
# Runs inside the rockbot-psp Docker image (packages already installed).
set -euo pipefail

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

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
# psp_dist (not PSP/) avoids case-insensitive FS collisions with ports/psp.
STAGE_DIR="$PACKAGES_DIR/psp_dist"
GAME_DIR="Rockbot${VERSION_NUMBER}"
OUT_ZIP="$PACKAGES_DIR/RockBot_PSP_${VERSION_NAME}.zip"
BUILD_LOG="$PACKAGES_DIR/psp-build.log"

echo ">> [docker] Rockbot PSP v${VERSION_NUMBER} (${VERSION_NAME})"

for required in \
    "$GAME_DATA_ROOT/fonts" \
    "$GAME_DATA_ROOT/shared" \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}" \
    "$PACKAGES_DIR/files/psp_icon.png" \
    "$PACKAGES_DIR/files/psp_background.png"
do
    [[ -e "$required" ]] || { echo "Error: missing $required"; exit 1; }
done

cd "$REPO_ROOT"
mkdir -p "$PACKAGES_DIR"
: > "$BUILD_LOG"

echo "🛠️ Cleaning..."
make -f Makefile.psp clean >>"$BUILD_LOG" 2>&1 || true
rm -f EBOOT.PBP PARAM.SFO rockbot_psp.elf rockbot_psp.prx rockbot_psp_strip.elf

echo "🛠️ Building EBOOT.PBP (-j${NCPU})..."
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

echo "📦 Packaging..."
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
( cd "$STAGE_DIR" && zip -qr "$OUT_ZIP" "$GAME_DIR" )

echo "✅ Build completed successfully"
echo "EBOOT: $REPO_ROOT/EBOOT.PBP"
echo "ZIP:   $OUT_ZIP"
echo "Copy $GAME_DIR/ to ms0:/PSP/GAME/ on the Memory Stick"
