#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./linux
mkdir ./linux
mkdir ./linux/rockbot
cp ./docs/README.LINUX.TXT ./linux/README.TXT
cp ../rockbot ./linux/rockbot
cp ../editor ./linux/rockbot
rsync -r --exclude=.svn ../data ./linux/rockbot
rm ./linux/rockbot/data/game*.sav
rm ./linux/rockbot/data/config*.sav

cd ./linux
tar -cvf ../Rockbot_Linux_$VERSIONNAME.tar rockbot
gzip ../Rockbot_Linux_$VERSIONNAME.tar
