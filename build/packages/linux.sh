#!/usr/bin/env bash

set -x
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number

cd ..
make clean
cd ..
qmake RockDroid.pro CONFIG+=linux
cd build
make


rm -r -f ./linux
mkdir ./linux
mkdir ./linux/$GAME_DIR
cp ./docs/README.LINUX.TXT ./linux/README.TXT
cp ../rockbot ./linux/$GAME_DIR
cp ../editor ./linux/$GAME_DIR

rsync -r --exclude=mp3 --exclude=.svn ../games/RockDroid$version_number ./linux/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./linux/$GAME_DIR
rsync -r --exclude=.svn ../shared ./linux/$GAME_DIR

rm ./linux/$GAME_DIR/data/game*.sav
rm ./linux/$GAME_DIR/data/config*.sav

cd ./linux
tar -cvf ../Rockbot_Linux_$VERSIONNAME.tar $GAME_DIR
gzip ../Rockbot_Linux_$VERSIONNAME.tar
