#!/bin/sh

set -x

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
DIR=./pocketgo
GAME_DIR=Rockbot$version_number

rm -r -f $DIR
mkdir $DIR
mkdir $DIR/$GAME_DIR
mkdir $DIR/$GAME_DIR/games
mkdir $DIR/$GAME_DIR/fonts
mkdir $DIR/$GAME_DIR/shared

cp ../rockbot_pocketgo $DIR/$GAME_DIR/


mkdir ./$DIR/$GAME_DIR/data
mkdir ./$DIR/$GAME_DIR/data/games

rsync -r --exclude=.svn ../fonts ./$DIR/$GAME_DIR/data
rsync -r --exclude=.svn ../shared ./$DIR/$GAME_DIR/data
rsync -r --exclude=.svn ../games/RockDroid$version_number ./$DIR/$GAME_DIR/data/games

cp $DIR/$GAME_DIR/data/images/faces/rockbot.png $DIR/$GAME_DIR/
cd $DIR
zip -r ../Rockbot_PocketGo_$VERSIONNAME.zip *
