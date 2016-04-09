#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./PSP
mkdir ./PSP
mkdir ./PSP/Rockbot2
mkdir ./PSP/Rockbot2/games
mkdir ./PSP/Rockbot2/fonts
mkdir ./PSP/Rockbot2/shared
cp ../../EBOOT.PBP ./PSP/Rockbot2/
rsync -r --exclude=.svn ../games ./PSP/Rockbot2
rsync -r --exclude=.svn ../fonts ./PSP/Rockbot2
rsync -r --exclude=.svn ../shared ./PSP/Rockbot2
rm ./PSP/Rockbot2/data/game*.sav
rm ./PSP/Rockbot2/data/config*.sav
cd ./PSP
zip -r ../Rockbot2_PSP_$VERSIONNAME.zip *
