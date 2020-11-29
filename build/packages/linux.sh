#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./linux
mkdir ./linux
mkdir ./linux/$GAME_DIR
cp ./docs/README.LINUX.TXT ./linux/README.TXT
cp ../rockbot ./linux/$GAME_DIR
cp ../editor ./linux/$GAME_DIR

rsync -r --exclude=.svn ../games/RockDroid$version_number ./linux/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./linux/$GAME_DIR
rsync -r --exclude=.svn ../shared ./linux/$GAME_DIR

rm ./linux/$GAME_DIR/data/game*.sav
rm ./linux/$GAME_DIR/data/config*.sav

cd ./linux
tar -cvf ../Rockbot_Linux_$VERSIONNAME.tar $GAME_DIR
gzip ../Rockbot_Linux_$VERSIONNAME.tar
