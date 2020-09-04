#!/bin/sh

set -x

VERSIONNAME=`cat version_name_v1.txt`

rm -r -f ./PS2
mkdir ./PS2
mkdir ./PS2/Rockbot
mkdir ./PS2/Rockbot/games
mkdir ./PS2/Rockbot/fonts
mkdir ./PS2/Rockbot/shared

cp ../rockbot.elf ./PS2/Rockbot/


mkdir ././PS2/Rockbot/data
mkdir ././PS2/Rockbot/data/games
rsync -r --exclude=.svn ../fonts ././PS2/Rockbot/data
rsync -r --exclude=.svn ../games/RockDroid1 ././PS2/Rockbot/data/games
rsync -r --exclude=.svn ../games/RockDroid2 ././PS2/Rockbot/data/games
rsync -r --exclude=.svn ../shared ././PS2/Rockbot/data

rsync -r --exclude=.svn ../games ./PS2/Rockbot
rsync -r --exclude=.svn ../fonts ./PS2/Rockbot
rsync -r --exclude=.svn ../shared ./PS2/Rockbot

cp ./rockbot_ps2_icon.icn ./PS2/Rockbot/shared/images/
cp ./icon.sys ./PS2/Rockbot/data/images/
cd ./PS2
zip -r ../Rockbot2_PS2_$VERSIONNAME.zip *
