#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

#copy data
rm -r -f ./win32/$GAME_DIR
mkdir -p ./win32/$GAME_DIR/games
rsync -r --exclude=.svn ../fonts ./win32/$GAME_DIR
rsync -r --exclude=.svn ../shared ./win32/$GAME_DIR
rsync -r --exclude=.svn ../games/Rockbot$version_number ./win32/$GAME_DIR/games
cp ./win32/*.dll ./win32/$GAME_DIR
cp ./win32/*.exe ./win32/$GAME_DIR
rm ./win32/$GAME_DIR/game*.sav
rm ./win32/$GAME_DIR/config_v*.sav


cd win32/$GAME_DIR
spacer="_WIN32_"
zip -r ../../Rockbot_Win32_$VERSIONNAME.zip *
