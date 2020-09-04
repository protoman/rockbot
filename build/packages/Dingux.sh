#!/bin/sh

set -x

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`

rm -r -f ./Dingux
mkdir ./Dingux
mkdir ./Dingux/Rockbot
mkdir ./Dingux/Rockbot/games
mkdir ./Dingux/Rockbot/fonts
mkdir ./Dingux/Rockbot/shared

cp ../rockbot.dge ./Dingux/Rockbot/


rsync -r ../fonts ././Dingux/Rockbot/
rsync -r --exclude=mp3 ../games/RockDroid$version_number ./Dingux/Rockbot/games
rsync -r ../shared ././Dingux/Rockbot/

rsync -r ../fonts ./Dingux/Rockbot
rsync -r ../shared ./Dingux/Rockbot

cp ./Dingux/Rockbot/games/RockDroid$version_number/images/faces/rockbot.png ./Dingux/Rockbot/
cd ./Dingux
zip -r ../Rockbot2_Dingux_$VERSIONNAME.zip *
