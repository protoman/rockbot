#!/bin/bash
set -e

# sdl1 
# ./build-ubuntu.sh

# sdl2
# ./build-ubuntu.sh sdl2


if [[ "${1^^}" == "SDL2" ]]; then
    echo ">> SDL2 mode enabled"
    USE_SDL2=true
else
    USE_SDL2=false
fi

# Set non-interactive mode for apt
export DEBIAN_FRONTEND=noninteractive

echo "🛠️ Updating packages and installing dependencies..."
sudo apt update && sudo apt install -y \
    build-essential \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libgl1-mesa-dev \


if $USE_SDL2; then
    sudo apt install -y libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-ttf-dev \
        libsdl2-mixer-dev \
        libsdl2-gfx-dev
else
    sudo apt install -y libsdl1.2-dev \
        libsdl-image1.2-dev \
        libsdl-ttf2.0-dev \
        libsdl-mixer1.2-dev \
        libsdl-gfx1.2-dev
fi

export QT_SELECT=qt5

echo "📁 Building rockbot the project..."
if $USE_SDL2; then
    qmake RockDroid.pro CONFIG=linux DESTDIR=build DEFINES+=SDL2 LIBS="-lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx `sdl2-config --libs`"
else
    qmake RockDroid.pro CONFIG=linux DESTDIR=build
fi
make clean build/rockbot

echo "📁 Building rockbot-editor the project..."
cd editor

if $USE_SDL2; then
    qmake Rockbot_Editor.pro CONFIG+=linux QMAKE_CCFLAGS+=-DSDL2 QMAKE_CXXFLAGS+=-DSDL2
else
    qmake Rockbot_Editor.pro CONFIG+=linux
fi

make
echo "✅ Build completed successfully"


echo "Usage Instructions:"
echo
echo "1. Enter the build directory:"
echo "   cd build"
echo
echo "2. Run the Editor:"
echo "   GTK_PATH= ./editor"
echo
echo "3. Run the Game:"
echo "   ./rockbot"