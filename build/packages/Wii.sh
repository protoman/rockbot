#!/usr/bin/env bash
# Nintendo Wii package for the Homebrew Channel: cross-builds boot.dol in Docker (devkitPro, see
# ../../build-wii.sh) and creates Rockbot<N>_Wii_<version>.zip (unzip into the SD card root).
# Usage: ./Wii.sh [1|2]     (asks which game if omitted)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

GAME_DIR="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_Wii_$VERSIONNAME.zip"
APP="./Wii/apps/$GAME_DIR"

../../build-wii.sh

rm -rf ./Wii
mkdir -p "$APP"
cp ../../build-wii/rockbot_wii.dol "$APP/boot.dol"
cp files/wii_meta/icon.png "$APP/icon.png"
cat > "$APP/meta.xml" <<META
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<app version="1">
  <name>Rockbot $version_number</name>
  <coder>Iuri Fiedoruk</coder>
  <version>$VERSIONNAME</version>
  <release_date>$(date +%Y%m%d%H%M%S)</release_date>
  <short_description>Rockbot game Wii port</short_description>
  <long_description>Rockbot is a game made in the spirit of classic Capcom Megaman (Rockman in Japan) game series (1-6 and 9-10), that have original storyline and graphics, besides an editor where the user can edit their own stage and enemies, allowing other people to distribute their own versions of the game.</long_description>
</app>
META
stage_game_data "$APP"

rm -f "$OUT"
( cd ./Wii && zip -qr "$OUT" . )
echo "Package: $OUT"
