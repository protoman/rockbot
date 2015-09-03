#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./Dingux
mkdir ./Dingux
mkdir ./Dingux/Rockbot
cp ../rockbot.dge ./Dingux/Rockbot/
rsync -r --exclude=.svn ../data ./Dingux/Rockbot/
rm ./Dingux/Rockbot/data/game*.sav
rm ./Dingux/Rockbot/data/config_*.sav

cp ./Dingux/Rockbot/data/images/faces/rockbot.png ./Dingux/Rockbot/
cd ./Dingux
zip -r ../Rockbot_Dingux_$VERSIONNAME.zip *
