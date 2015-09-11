#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./win32/rockbot/data
rsync -r --exclude=.svn ../../build/data ./win32/rockbot/
rm ./win32/rockbot/data/game*.sav
rm ./win32/rockbot/data/config*.sav

cd win32
zip -r ../Rockbot_WIN32_$VERSIONNAME.zip *
