#!/bin/sh
read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./source
mkdir ./source
rsync -r --exclude=.svn --exclude=build ../.. ./source
mkdir ./source/build
rsync -r --exclude=.svn ../../build/data ./source/build
rm ./source/build/data/game*.sav
rm ./source/build/data/config*.sav

tar -cvf ./Rockbot_Source_$VERSIONNAME.tar ./source
gzip ./Rockbot_Source_$VERSIONNAME.tar
