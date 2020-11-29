#!/bin/sh

set -x

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./Dingux
mkdir ./Dingux
mkdir ./Dingux/$GAME_DIR
mkdir ./Dingux/$GAME_DIR/games
mkdir ./Dingux/$GAME_DIR/fonts
mkdir ./Dingux/$GAME_DIR/shared

cp ../rockbot.dge ./Dingux/$GAME_DIR/


rsync -r ../fonts ././Dingux/$GAME_DIR/
rsync -r --exclude=mp3 ../games/RockDroid$version_number ./Dingux/$GAME_DIR/games
rsync -r ../shared ././Dingux/$GAME_DIR/

rsync -r ../fonts ./Dingux/$GAME_DIR
rsync -r ../shared ./Dingux/$GAME_DIR

cp ./Dingux/$GAME_DIR/games/RockDroid$version_number/images/faces/rockbot.png ./Dingux/$GAME_DIR/
cd ./Dingux
zip -r ../Rockbot2_Dingux_$VERSIONNAME.zip *
