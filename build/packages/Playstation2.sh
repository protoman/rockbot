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

rm -r -f ./PS2
mkdir ./PS2
mkdir ./PS2/$GAME_DIR
mkdir ./PS2/$GAME_DIR/games
mkdir ./PS2/$GAME_DIR/fonts
mkdir ./PS2/$GAME_DIR/shared

cp ../rockbot.elf ./PS2/$GAME_DIR/


mkdir ././PS2/$GAME_DIR/data
mkdir ././PS2/$GAME_DIR/data/games
rsync -r --exclude=.svn ../fonts ././PS2/$GAME_DIR/data
rsync -r --exclude=.svn ../shared ././PS2/$GAME_DIR/data
rsync -r --exclude=.svn ../games/RockDroid$version_number ./PS2/$GAME_DIR/data/games

rsync -r --exclude=.svn ../games ./PS2/$GAME_DIR
rsync -r --exclude=.svn ../fonts ./PS2/$GAME_DIR
rsync -r --exclude=.svn ../shared ./PS2/$GAME_DIR

cp ./rockbot_ps2_icon.icn ./PS2/$GAME_DIR/shared/images/
cp ./icon.sys ./PS2/$GAME_DIR/data/images/
cd ./PS2
zip -r ../Rockbot2_PS2_$VERSIONNAME.zip *
