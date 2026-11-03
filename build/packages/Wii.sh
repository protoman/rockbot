#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number

rm -r -f ./Wii
mkdir ./Wii
mkdir ./Wii/apps
mkdir ./Wii/apps/$GAME_DIR
#unzip unzip -x ./files/wii_meta.zip -d ./Wii/apps/$GAME_DIR
cp ./files/wii_meta/* ./Wii/apps/$GAME_DIR
cp ../boot.dol ./Wii/apps/$GAME_DIR

rsync -r --exclude=.svn ../fonts ./Wii/apps/$GAME_DIR
rsync -r --exclude=.svn ../shared ./Wii/apps/$GAME_DIR
rsync -r --exclude=mp3 --exclude=.svn ../games/RockDroid$version_number ./Wii/apps/$GAME_DIR/games

cd ./Wii
zip -r ../Rockbot_Wii_$VERSIONNAME.zip *
