#!/bin/sh
cd ../..
VERSIONNAME=`cat version_name_v1.txt`

rm -r -f ./PSP
mkdir ./PSP
mkdir ./PSP/Rockbot1
mkdir ./PSP/Rockbot1/games
mkdir ./PSP/Rockbot1/fonts
mkdir ./PSP/Rockbot1/shared
cp ../../EBOOT.PBP ./PSP/Rockbot1/
rsync -r --exclude=.svn ../games/Rockbot1 ./PSP/Rockbot1/games/
rsync -r --exclude=.svn ../fonts ./PSP/Rockbot1
rsync -r --exclude=.svn ../shared ./PSP/Rockbot1
rm ./PSP/Rockbot1/data/game*.sav
rm ./PSP/Rockbot1/data/config*.sav
cd ./PSP
zip -r ../Rockbot1_PSP_$VERSIONNAME.zip *
