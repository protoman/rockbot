#!/bin/sh
read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

set -x

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=Rockbot$version_number

rm -r -f ./PSP/$GAME_DIR

#cd ..
#make clean
#cd ..
#qmake ../RockDroid.pro CONFIG -=linux CONFIG+=psp
#cd build
#make
#cd packages

cd ../..
pwd
make -f Makefile.psp clean
make -f Makefile.psp version_number=$version_number VERSIONNAME=$VERSIONNAME

rm -r -f ./PSP
mkdir ./PSP
mkdir ./PSP/$GAME_DIR
mkdir ./PSP/$GAME_DIR/games
mkdir ./PSP/$GAME_DIR/fonts
mkdir ./PSP/$GAME_DIR/shared
cp ./EBOOT.PBP ./PSP/$GAME_DIR/
rsync -r --exclude=mp3 --exclude=.svn ./build/games/RockDroid$version_number ./PSP/$GAME_DIR/games
rsync -r --exclude=.svn ./build/fonts ./PSP/$GAME_DIR
rsync -r --exclude=.svn ./build/shared ./PSP/$GAME_DIR
rm ./PSP/$GAME_DIR/data/game*.sav
rm ./PSP/$GAME_DIR/data/config*.sav
cd ./PSP
zip -r ../Rockbot2_PSP_$VERSIONNAME.zip *
