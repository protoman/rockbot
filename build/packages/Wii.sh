#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./Wii
mkdir ./Wii
mkdir ./Wii/apps
mkdir ./Wii/apps/$GAME_DIR
#unzip unzip -x ./files/wii_meta.zip -d ./Wii/apps/$GAME_DIR
cp ./files/wii_meta/* ./Wii/apps/$GAME_DIR
cp ../boot.dol ./Wii/apps/$GAME_DIR
rsync -r --exclude=.svn ../fonts ./Wii/apps/$GAME_DIR
rsync -r --exclude=.svn ../games ./Wii/apps/$GAME_DIR
rsync -r --exclude=.svn ../shared ./Wii/apps/$GAME_DIR
cd ./Wii
zip -r ../Rockbot_Wii_$VERSIONNAME.zip *
