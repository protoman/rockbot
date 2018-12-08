#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./Dingux
mkdir ./Dingux
mkdir ./Dingux/Rockbot2
mkdir ./Dingux/Rockbot2
mkdir ./Dingux/Rockbot2/games
mkdir ./Dingux/Rockbot2/fonts
mkdir ./Dingux/Rockbot2/shared

cp ../rockbot.dge ./Dingux/Rockbot2/


mkdir ././Dingux/Rockbot2/data
mkdir ././Dingux/Rockbot2/data/games
rsync -r --exclude=.svn ../fonts ././Dingux/Rockbot2/data
rsync -r --exclude=.svn ../games/Rockbot$version_number ././Dingux/Rockbot2/data/games
rsync -r --exclude=.svn ../shared ././Dingux/Rockbot2/data

rsync -r --exclude=.svn ../games ./Dingux/Rockbot2
rsync -r --exclude=.svn ../fonts ./Dingux/Rockbot2
rsync -r --exclude=.svn ../shared ./Dingux/Rockbot2

cp ./Dingux/Rockbot2/data/images/faces/rockbot.png ./Dingux/Rockbot2/
cd ./Dingux
zip -r ../Rockbot2_Dingux_$VERSIONNAME.zip *
