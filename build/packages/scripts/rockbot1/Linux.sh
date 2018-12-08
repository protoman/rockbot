#!/bin/sh
cd ../..
VERSIONNAME=`cat version_name_v1.txt`


rm -r -f ./linux
mkdir ./linux
mkdir ./linux/rockbot
cp ./docs/README.LINUX.TXT ./linux/README.TXT
cp ../rockbot ./linux/rockbot
cp ../editor ./linux/rockbot

mkdir ./linux/rockbot/games
rsync -r --exclude=.svn ../games/Rockbot1 ./linux/rockbot/games
rsync -r --exclude=.svn ../shared ./linux/rockbot/
rsync -r --exclude=.svn ../fonts ./linux/rockbot/

cd ./linux
tar -cvf ../Rockbot_Linux_$VERSIONNAME.tar rockbot
gzip ../Rockbot_Linux_$VERSIONNAME.tar
