#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./source
mkdir ./source
rsync -r --exclude=.svn --exclude=build ../.. ./source
mkdir ./source/build
rsync -r --exclude=.svn ../../build/data ./source/build
rm ./source/build/data/game*.sav
rm ./source/build/data/config*.sav

tar -cvf ./Rockbot_Source_$VERSIONNAME.tar ./source
gzip ./Rockbot_Source_$VERSIONNAME.tar
