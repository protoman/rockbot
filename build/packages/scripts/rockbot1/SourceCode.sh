#!/bin/sh
cd ../..
VERSIONNAME=`cat version_name_v1.txt`

rm -r -f ./source
mkdir ./source
rsync -r --exclude=.svn --exclude=build ../.. ./source
mkdir ./source/build
mkdir ./source/build/games
rsync -r --exclude=.svn ../../build/games/Rockbot1 ./source/build/games
rsync -r --exclude=.svn ../../build/shared ./source/build
rsync -r --exclude=.svn ../../build/fonts ./source/build

tar -cvf ./Rockbot_Source_$VERSIONNAME.tar ./source
gzip ./Rockbot_Source_$VERSIONNAME.tar
