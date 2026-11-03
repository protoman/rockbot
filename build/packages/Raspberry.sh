#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number

rm -r -f ./raspberry
mkdir ./raspberry
mkdir ./raspberry/$GAME_DIR
cp ../rockbot_pi ./raspberry/$GAME_DIR/rockbot

rsync -r --exclude=mp3 --exclude=.svn ../games/RockDroid$version_number ./raspberry/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./raspberry/$GAME_DIR
rsync -r --exclude=.svn ../shared ./raspberry/$GAME_DIR

rm ./raspberry/$GAME_DIR/data/game*.sav
rm ./raspberry/$GAME_DIR/data/config*.sav

cd ./raspberry
tar -cvf ../Rockbot_Raspberry_$VERSIONNAME.tar $GAME_DIR
gzip ../Rockbot_Raspberry_$VERSIONNAME.tar
