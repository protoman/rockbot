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

rm -r -f $DIR
mkdir $DIR
mkdir $DIR/Rockbot
mkdir $DIR/Rockbot/games
mkdir $DIR/Rockbot/fonts
mkdir $DIR/Rockbot/shared

cp ../rockbot_pocketgo $DIR/Rockbot/


mkdir ./$DIR/Rockbot/data
mkdir ./$DIR/Rockbot/data/games
rsync -r --exclude=.svn ../fonts ./$DIR/Rockbot/data
#rsync -r --exclude=.svn ../games/RockDroid$version_number ./$DIR/Rockbot/data/games
rsync -r --exclude=.svn ../shared ./$DIR/Rockbot/data

rsync -r --exclude=.svn ../games $DIR/Rockbot
rsync -r --exclude=.svn ../fonts $DIR/Rockbot
rsync -r --exclude=.svn ../shared $DIR/Rockbot

cp $DIR/Rockbot/data/images/faces/rockbot.png $DIR/Rockbot/
cd $DIR
zip -r ../Rockbot2_PocketGo_$VERSIONNAME.zip *
