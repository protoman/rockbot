#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

set -x

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so

INITIAL_PATH=`pwd`

cd rockdroid/build
make clean
qmake "CONFIG += android" ../RockDroid.pro
make
cp ./libapplication-armeabi.so $INITIAL_PATH/../src/
cd $INITIAL_PATH
