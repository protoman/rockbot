#!/bin/sh

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./pandora/$GAME_DIR
mkdir ./pandora
mkdir ./pandora/$GAME_DIR
#cp ./docs/README.LINUX.TXT ./pandora/README.TXT
#cp _$VERSIONNAME.tar../../$GAME_DIR ./pandora/$GAME_DIR
#cp ../../editor ./pandora/$GAME_DIR
cp -r ./pandora/project/* ./pandora/$GAME_DIR
cp ../rockbot_pandora ./pandora/$GAME_DIR/rockbot
rsync -r --exclude=.svn ../data ./pandora/$GAME_DIR
rm ./pandora/$GAME_DIR/data/game*.sav
rm ./pandora/$GAME_DIR/data/config*.sav
cd ./pandora
/arquivos/development/SDK/OpenPandora/scripts/pnd_make -p ../Rockbot_Pandora_$VERSIONNAME.pnd -d ./rockbot -x ./rockbot/pxml.xml
