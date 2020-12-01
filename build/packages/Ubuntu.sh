#/usr/bin

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`
GAME_DIR=rockbot$version_number

rm -r -f ./ubuntu/game/debian/opt/
mkdir ./ubuntu/game/debian/opt/
mkdir ./ubuntu
mkdir ./ubuntu/game
cd ./ubuntu/game
rm -r -f ./debian/opt/game/$GAME_DIR
mkdir -p ./debian/opt/game/$GAME_DIR/games

rsync -r --exclude=.svn ../../../fonts ./debian/opt/game/$GAME_DIR
rsync -r --exclude=.svn ../../../shared ./debian/opt/game/$GAME_DIR
rsync -r --exclude=mp3 --exclude=.svn ../../../games/RockDroid$version_number ./debian/opt/game/$GAME_DIR/games

rm ./debian/opt/game/$GAME_DIR/game*.sav
rm ./debian/opt/game/$GAME_DIR/config*.sav

cp ../../../rockbot ./debian/opt/game/$GAME_DIR/
cp ../../../editor ./debian/opt/game/$GAME_DIR/
mkdir -p ./debian/DEBIAN
cp ../../control ./debian/DEBIAN

spacer="_Ubuntu_"
dpkg-deb --build debian/ ../../Rockbot_Ubuntu_$VERSIONNAME.deb
