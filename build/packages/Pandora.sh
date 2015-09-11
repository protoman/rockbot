#!/bin/sh
VERSIONNAME=`cat version_name.txt`

rm -r -f ./pandora/rockbot
mkdir ./pandora
mkdir ./pandora/rockbot
#cp ./docs/README.LINUX.TXT ./pandora/README.TXT
#cp _$VERSIONNAME.tar../../rockbot ./pandora/rockbot
#cp ../../editor ./pandora/rockbot
cp -r ./pandora/project/* ./pandora/rockbot
cp ../rockbot_pandora ./pandora/rockbot/rockbot
rsync -r --exclude=.svn ../data ./pandora/rockbot
rm ./pandora/rockbot/data/game*.sav
rm ./pandora/rockbot/data/config*.sav
cd ./pandora
/arquivos/development/SDK/OpenPandora/scripts/pnd_make -p ../Rockbot_Pandora_$VERSIONNAME.pnd -d ./rockbot -x ./rockbot/pxml.xml
