#!/bin/sh

VERSIONNAME=`cat version_name.txt`

rm -r -f ./Wii
mkdir ./Wii
mkdir ./Wii/apps
mkdir ./Wii/apps/rockbot
#unzip unzip -x ./files/wii_meta.zip -d ./Wii/apps/rockbot
cp ./files/wii_meta/* ./Wii/apps/rockbot
cp ../boot.dol ./Wii/apps/rockbot
rsync -r --exclude=.svn ../fonts ./Wii/apps/rockbot
rsync -r --exclude=.svn ../games ./Wii/apps/rockbot
rsync -r --exclude=.svn ../shared ./Wii/apps/rockbot
cd ./Wii
zip -r ../Rockbot_Wii_$VERSIONNAME.zip *
