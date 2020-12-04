#!/bin/sh
read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=Rockbot$version_number

rm -r -f ./PSP
mkdir ./PSP
mkdir ./PSP/$GAME_DIR
mkdir ./PSP/$GAME_DIR/games
mkdir ./PSP/$GAME_DIR/fonts
mkdir ./PSP/$GAME_DIR/shared
cp ../EBOOT.PBP ./PSP/$GAME_DIR/
rsync -r --exclude=mp3 --exclude=.svn ../games/RockDroid$version_number ./PSP/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./PSP/$GAME_DIR
rsync -r --exclude=.svn ../shared ./PSP/$GAME_DIR
rm ./PSP/$GAME_DIR/data/game*.sav
rm ./PSP/$GAME_DIR/data/config*.sav
cd ./PSP
zip -r ../Rockbot2_PSP_$VERSIONNAME.zip *
