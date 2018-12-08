#/usr/bin
cd ../..
VERSIONNAME=`cat version_name_v1.txt`

rm -r -f ./ubuntu/game/debian/opt/
mkdir ./ubuntu/game/debian/opt/
mkdir ./ubuntu
mkdir ./ubuntu/game
cd ./ubuntu/game
rm -r -f ./debian/opt/game/rockbot1
mkdir -p ./debian/opt/game/rockbot1/games

rsync -r --exclude=.svn ../../../fonts ./debian/opt/game/rockbot1
rsync -r --exclude=.svn ../../../shared ./debian/opt/game/rockbot1
rsync -r --exclude=.svn ../../../games/Rockbot1 ./debian/opt/game/rockbot1/games

cp ../../../rockbot ./debian/opt/game/rockbot1/
cp ../../../editor ./debian/opt/game/rockbot1/
mkdir -p ./debian/DEBIAN
cp ../../control ./debian/DEBIAN

spacer="_Debian_"
dpkg-deb --build debian/ ../../Rockbot1$spacer$VERSIONNAME.deb
