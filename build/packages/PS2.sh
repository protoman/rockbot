#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm ./Rockbot_PS2_$VERSIONNAME.zip
rm -r -f ./PS2
mkdir ./PS2
mkdir ./PS2/Rockbot

cp ../rockbot.elf ./PS2/Rockbot/

rsync -r --exclude=.svn ../fonts ./PS2/Rockbot/
rsync -r --exclude=.svn ../shared ./PS2/Rockbot/
rsync -r --exclude=.svn ../games ./PS2/Rockbot/
rsync -r --exclude=.svn ../*.dat ./PS2/Rockbot/

cp ./rockbot_ps2_icon.icn ./PS2/Rockbot/data/images/
cp ./icon.sys ./PS2/Rockbot/data/images/
rm ./PS2/Rockbot/data/game*.sav
rm ./PS2/Rockbot/data/config*.sav

cd ./PS2
zip -r ../Rockbot_PS2_$VERSIONNAME.zip *
