#!/bin/bash
set -e
set -o pipefail

# Usage:
#   ./build-osx.sh          → SDL2 (default)
#   ./build-osx.sh sdl2     → SDL2
#   ./build-osx.sh sdl3     → SDL3

MODE="$(printf '%s' "${1:-sdl2}" | tr '[:upper:]' '[:lower:]')"
case "$MODE" in
    sdl2)
        USE_SDL3=false
        echo ">> SDL2 mode enabled"
        ;;
    sdl3)
        USE_SDL3=true
        echo ">> SDL3 mode enabled"
        ;;
    *)
        echo "Usage: $0 [sdl2|sdl3]"
        exit 1
        ;;
esac

echo "🛠️ Installing dependencies with Homebrew..."
brew update
brew install \
    qt@5 \
    gnu-sed

if $USE_SDL3; then
    brew install \
        sdl3 \
        sdl3_image \
        sdl3_ttf \
        sdl3_mixer
else
    brew install \
        sdl2 \
        sdl2_image \
        sdl2_ttf \
        sdl2_mixer \
        sdl2_gfx
fi

export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
export QT_SELECT=qt5

# Resolve the macOS SDK actually installed (e.g. macosx26.5).
# RockDroid.pro may hardcode an older QMAKE_MAC_SDK that Xcode no longer ships.
MAC_SDK_VERSION="$(xcrun --sdk macosx --show-sdk-version)"
MAC_SDK="macosx${MAC_SDK_VERSION}"
echo "📦 Using macOS SDK: ${MAC_SDK}"

echo "📁 Building rockbot the project..."
gsed -i 's/^CONFIG += linux/#CONFIG += linux/g' RockDroid.pro
gsed -i "s/^[[:space:]]*QMAKE_MAC_SDK *= *.*/    QMAKE_MAC_SDK = ${MAC_SDK}/" RockDroid.pro

# RockDroid.pro macosx block hardcodes SDL linkage; normalize for the selected mode.
if $USE_SDL3; then
    gsed -i \
        -e 's/LIBS += `\(sdl2-config --libs\|pkg-config --libs sdl3\)`/LIBS += `pkg-config --libs sdl3`/' \
        -e 's/LIBS += -lSDL[23]_mixer -lSDL[23]_image -lSDL[23]_ttf\( -lSDL2_gfx\)\?/LIBS += -lSDL3_mixer -lSDL3_image -lSDL3_ttf/' \
        -e 's/INCLUDES = -I\/opt\/homebrew\/include -I\/opt\/homebrew\/opt\/qt@5 `[^`]*` -I\./INCLUDES = -I\/opt\/homebrew\/include -I\/opt\/homebrew\/opt\/qt@5 `pkg-config --cflags sdl3` -I./' \
        RockDroid.pro

    SDL_DEFINES="DEFINES+=SDL3"
    SDL_CFLAGS="QMAKE_CCFLAGS+=-DSDL3"
    SDL_CXXFLAGS="QMAKE_CXXFLAGS+=-DSDL3"
    SDL_LIBS="-lSDL3_mixer -lSDL3_image -lSDL3_ttf $(pkg-config --libs sdl3)"
else
    gsed -i \
        -e 's/LIBS += `\(sdl2-config --libs\|pkg-config --libs sdl3\)`/LIBS += `sdl2-config --libs`/' \
        -e 's/LIBS += -lSDL[23]_mixer -lSDL[23]_image -lSDL[23]_ttf\( -lSDL2_gfx\)\?/LIBS += -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx/' \
        -e 's/INCLUDES = -I\/opt\/homebrew\/include -I\/opt\/homebrew\/opt\/qt@5 `[^`]*` -I\./INCLUDES = -I\/opt\/homebrew\/include -I\/opt\/homebrew\/opt\/qt@5 `sdl2-config --cflags` -I./' \
        RockDroid.pro

    SDL_DEFINES="DEFINES+=SDL2"
    SDL_CFLAGS="QMAKE_CCFLAGS+=-DSDL2"
    SDL_CXXFLAGS="QMAKE_CXXFLAGS+=-DSDL2"
    SDL_LIBS="-lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx $(sdl2-config --libs)"
fi

qmake RockDroid.pro \
    CONFIG=macosx \
    CONFIG+=sdk_no_version_check \
    DESTDIR=build \
    ${SDL_DEFINES} \
    ${SDL_CFLAGS} \
    ${SDL_CXXFLAGS} \
    "QMAKE_MAC_SDK=${MAC_SDK}" \
    "LIBS=${SDL_LIBS}"

gsed -i 's/-mmacosx-version-min=[0-9.]*/-mmacosx-version-min=10.7/g' Makefile
rm -f build/rockbot
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
# rm -f build/editor
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
