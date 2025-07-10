#!/bin/bash
set -e

echo "🛠️ Installing dependencies with Homebrew..."
brew update
brew install \
    qt@5 \
    sdl2 \
    sdl2_image \
    sdl2_ttf \
    sdl2_mixer \
    sdl2_gfx

export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
export QT_SELECT=qt5

echo "📁 Building rockbot the project..."
qmake RockDroid.pro CONFIG=macosx DESTDIR=build DEFINES+=SDL2 QMAKE_CCFLAGS+=-DSDL2 QMAKE_CXXFLAGS+=-DSDL2
gsed -i 's/-mmacosx-version-min=[0-9.]*/-mmacosx-version-min=10.7/g' Makefile
make clean build/rockbot

# echo "📁 Building rockbot-editor the project..."
# cd editor
# qmake Rockbot_Editor.pro CONFIG=macosx DEFINES+=SDL2
# gsed -i 's/-mmacosx-version-min=[0-9.]*/-mmacosx-version-min=10.7/g' Makefile
# for f in $(find . -name '*.ui'); do
#   base=$(basename "$f" .ui);
#   echo "Generating ui_${base}.h from $f"; 
#   uic "$f" -o "ui_${base}.h";
# done
# make

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
