#/usr/bin

read -r -p "What is the version number you are building? " version_number
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ];
then
    echo "Invalid version number"
    exit
fi

VERSIONNAME=`cat version_name_v$version_number.txt`

rm -r -f ./ubuntu/game/debian/opt/
mkdir ./ubuntu/game/debian/opt/
mkdir ./ubuntu
mkdir ./ubuntu/game
cd ./ubuntu/game
rm -r -f ./debian/opt/game/rockbot$version_number
mkdir -p ./debian/opt/game/rockbot$version_number/games

rsync -r --exclude=.svn ../../../fonts ./debian/opt/game/rockbot$version_number
rsync -r --exclude=.svn ../../../shared ./debian/opt/game/rockbot$version_number
rsync -r --exclude=.svn ../../../games/RockDroid$version_number ./debian/opt/game/rockbot$version_number/games

rm ./debian/opt/game/rockbot$version_number/game*.sav
rm ./debian/opt/game/rockbot$version_number/config*.sav

cp ../../../rockbot ./debian/opt/game/rockbot$version_number/
cp ../../../editor ./debian/opt/game/rockbot$version_number/
mkdir -p ./debian/DEBIAN
cp ../../control ./debian/DEBIAN

spacer="_Ubuntu_"
dpkg-deb --build debian/ ../../Rockbot$version_number$spacer$VERSIONNAME.deb
