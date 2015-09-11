#/usr/bin
VERSIONNAME=`cat version_name.txt`

rm -r -f ./ubuntu/debian/opt/rockbot
mkdir ./ubuntu
mkdir ./ubuntu/game
cd ./ubuntu/game
rm -r -f ./debian/opt/rockbot
mkdir -p ./debian/opt/rockbot
rm -r -f ./debian/opt/rockbot/data
rsync -r --exclude=.svn ../../../data ./debian/opt/rockbot
rm ./debian/opt/rockbot/data/game*.sav
rm ./debian/opt/rockbot/data/config*.sav

cp ../../../rockbot ./debian/opt/rockbot/
cp ../../../editor ./debian/opt/rockbot/
cp ../../../conversor ./debian/opt/rockbot/
mkdir -p ./debian/DEBIAN
cp ../../control ./debian/DEBIAN

dpkg-deb --build debian/ ../../Rockbot_Ubuntu_$VERSIONNAME.deb
