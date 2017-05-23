#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`

#copy data
rm -r -f ./win32/rockbot
mkdir -p ./win32/rockbot/games
rsync -r --exclude=.svn ../fonts ./win32/rockbot
rsync -r --exclude=.svn ../shared ./win32/rockbot
rsync -r --exclude=.svn ../games/Rockbot$version_number ./win32/rockbot/games
cp ./win32/*.dll ./win32/rockbot
cp ./win32/*.exe ./win32/rockbot
rm ./win32/rockbot/game*.sav
rm ./win32/rockbot/config_v*.sav


cd win32/rockbot
spacer="_WIN32_"
zip -r ../../Rockbot$version_number$spacer$VERSIONNAME.zip *
