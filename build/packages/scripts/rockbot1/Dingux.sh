#!/bin/sh
cd ../..
VERSIONNAME=`cat version_name_v1.txt`

rm -r -f ./Dingux
mkdir ./Dingux
mkdir ./Dingux/Rockbot1
mkdir ./Dingux/Rockbot1/games
mkdir ./Dingux/Rockbot1/fonts
mkdir ./Dingux/Rockbot1/shared

cp ../rockbot.dge ./Dingux/Rockbot1/


mkdir ./Dingux/Rockbot1/data
mkdir ./Dingux/Rockbot1/games/Rockbot1
rsync -r --exclude=.svn ../fonts ././Dingux/Rockbot1
rsync -r --exclude=.svn ../games/Rockbot1 ./Dingux/Rockbot1/games
rsync -r --exclude=.svn ../shared ././Dingux/Rockbot1

cp ../shared/images/window_icon.png ./Dingux/Rockbot1/rockbot.png
cd ./Dingux
zip -r ../Rockbot2_Dingux_$VERSIONNAME.zip *
