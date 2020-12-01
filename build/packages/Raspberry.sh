#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./raspberry
mkdir ./raspberry
mkdir ./raspberry/$GAME_DIR
cp ../rockbot ./raspberry/$GAME_DIR
cp ../editor ./raspberry/$GAME_DIR

rsync -r --exclude=mp3 --exclude=.svn ../games/RockDroid$version_number ./raspberry/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./raspberry/$GAME_DIR
rsync -r --exclude=.svn ../shared ./raspberry/$GAME_DIR

rm ./raspberry/$GAME_DIR/data/game*.sav
rm ./raspberry/$GAME_DIR/data/config*.sav

cd ./raspberry
tar -cvf ../Rockbot_Raspberry_$VERSIONNAME.tar $GAME_DIR
gzip ../Rockbot_Raspberry_$VERSIONNAME.tar
