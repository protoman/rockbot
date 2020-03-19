#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`

rm -r -f ./linux
mkdir ./linux
mkdir ./linux/rockbot
cp ./docs/README.LINUX.TXT ./linux/README.TXT
cp ../rockbot ./linux/rockbot
cp ../editor ./linux/rockbot

rsync -r --exclude=.svn ../fonts ./linux/rockbot
rsync -r --exclude=.svn ../games/RockDroid$version_number ./linux/rockbot/games
rsync -r --exclude=.svn ../shared ./linux/rockbot

rm ./linux/rockbot/data/game*.sav
rm ./linux/rockbot/data/config*.sav

cd ./linux
tar -cvf ../Rockbot_Linux_$VERSIONNAME.tar rockbot
gzip ../Rockbot_Linux_$VERSIONNAME.tar
