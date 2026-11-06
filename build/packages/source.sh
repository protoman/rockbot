#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number

rm -r -f ./source
mkdir ./source
rsync -r --exclude=.AndroidData --exclude=.git --exclude=build ../.. ./source 
mkdir ./source/build
rsync -r --exclude=.git ../../build/data ./source/build
rm ./source/build/data/game*.sav
rm ./source/build/data/config*.sav

tar -cvf ./Rockbot_Source_$VERSIONNAME.tar ./source
gzip ./Rockbot_Source_$VERSIONNAME.tar
