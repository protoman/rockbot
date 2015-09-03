#!/bin/sh
VERSIONNAME=`cat version_name.txt`

cd ../game
make clean
make -f Makefile.ps2 clean
make -f Makefile.nds clean
make -f Makefile.nds
cd ../packages
rm -r -f ./NDS
mkdir ./NDS
mkdir ./NDS/Apps
mkdir ./NDS/Apps/Rockbot
cp ../game/rockbot.nds ./NDS/Apps/Rockbot/
sh ../bin/pack_data.sh
cp -r ./data ./NDS/Apps/Rockbot
cd ./NDS
zip -r ../Rockbot_NDS.zip *
