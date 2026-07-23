#!/bin/bash
# Runs inside the rockbot-switch Docker image (packages already installed).
set -euo pipefail

REPO_ROOT="${REPO_ROOT:-/rockbot}"
PACKAGES_DIR="$REPO_ROOT/build/packages"
GAME_DATA_ROOT="$REPO_ROOT/build"
VERSION_NUMBER="${VERSION_NUMBER:-${1:-}}"
NCPU="${NCPU:-$(nproc 2>/dev/null || echo 2)}"

if [[ "$VERSION_NUMBER" != "1" && "$VERSION_NUMBER" != "2" ]]; then
    echo "Usage (container): VERSION_NUMBER=1|2 rockbot-switch-build"
    exit 1
fi

export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITA64="${DEVKITA64:-$DEVKITPRO/devkitA64}"
export PATH="${DEVKITPRO}/tools/bin:${DEVKITA64}/bin:${PATH}"

VERSION_NAME="$(tr -d '[:space:]' < "$PACKAGES_DIR/version_name_v${VERSION_NUMBER}.txt")"
STAGE_DIR="$PACKAGES_DIR/switch_dist"
GAME_DIR="Rockbot${VERSION_NUMBER}"
OUT_ZIP="$PACKAGES_DIR/RockBot_Switch_${VERSION_NAME}.zip"
BUILD_LOG="$PACKAGES_DIR/switch-build.log"

echo ">> [docker] Rockbot Switch v${VERSION_NUMBER} (${VERSION_NAME})"

for required in \
    "$GAME_DATA_ROOT/fonts" \
    "$GAME_DATA_ROOT/shared" \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}"
do
    [[ -e "$required" ]] || { echo "Error: missing $required"; exit 1; }
done

ICON_CANDIDATE="$PACKAGES_DIR/files/switch_icon${VERSION_NUMBER}.jpg"
ICON_FALLBACK="$PACKAGES_DIR/files/switch_icon.jpg"
if [[ ! -f "$ICON_CANDIDATE" && ! -f "$ICON_FALLBACK" ]]; then
    echo "Warning: no Switch icon JPEG under build/packages/files/switch_icon*.jpg (hbmenu will show blank)"
fi

cd "$REPO_ROOT"
mkdir -p "$PACKAGES_DIR"
: > "$BUILD_LOG"

echo "🛠️ Cleaning..."
make -f Makefile.switch clean >>"$BUILD_LOG" 2>&1 || true
rm -f rockbot.nro rockbot.nacp rockbot.elf

echo "🛠️ Building rockbot.nro (-j${NCPU})..."
set +e
make -f Makefile.switch -j"$NCPU" \
    VERSION_NUMBER="$VERSION_NUMBER" \
    VERSIONNAME="$VERSION_NAME" \
    2>&1 | tee -a "$BUILD_LOG"
BUILD_STATUS=${PIPESTATUS[0]}
set -e

if [[ "$BUILD_STATUS" -ne 0 || ! -f "$REPO_ROOT/rockbot.nro" ]]; then
    echo "Error: Switch build failed (exit $BUILD_STATUS). Last 80 lines:"
    tail -n 80 "$BUILD_LOG" || true
    exit "${BUILD_STATUS:-1}"
fi

echo "📦 Packaging..."
rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/$GAME_DIR/games"

cp -f "$REPO_ROOT/rockbot.nro" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/fonts" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' "$GAME_DATA_ROOT/shared" "$STAGE_DIR/$GAME_DIR/"
rsync -a --exclude='.svn' --exclude='mp3' \
    "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}" \
    "$STAGE_DIR/$GAME_DIR/games/"

if [[ -f "$GAME_DATA_ROOT/games/RockDroid${VERSION_NUMBER}/images/icon_32px.png" ]]; then
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
echo "NRO: $REPO_ROOT/rockbot.nro"
echo "ZIP: $OUT_ZIP"
echo "Copy $GAME_DIR/ to sdmc:/switch/ (keep fonts/, shared/, games/ next to rockbot.nro)"
