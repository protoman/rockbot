#!/bin/bash
set -e

# Usage:
#   ./build-ubuntu.sh          → SDL1 (default)
#   ./build-ubuntu.sh sdl1     → SDL1
#   ./build-ubuntu.sh sdl2     → SDL2
#   ./build-ubuntu.sh sdl3     → SDL3

MODE="$(printf '%s' "${1:-sdl1}" | tr '[:upper:]' '[:lower:]')"
case "$MODE" in
    sdl1)
        SDL_MODE=sdl1
        echo ">> SDL1 mode enabled"
        ;;
    sdl2)
        SDL_MODE=sdl2
        echo ">> SDL2 mode enabled"
        ;;
    sdl3)
        SDL_MODE=sdl3
        echo ">> SDL3 mode enabled"
        ;;
    *)
        echo "Usage: $0 [sdl1|sdl2|sdl3]"
        exit 1
        ;;
esac

# Set non-interactive mode for apt
export DEBIAN_FRONTEND=noninteractive

echo "🛠️ Updating packages and installing dependencies..."
sudo apt update && sudo apt install -y \
    build-essential \
    cmake \
    curl \
    ca-certificates \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libgl1-mesa-dev \
    pkg-config \
    zip

# libsdl3-mixer-dev is not yet in Ubuntu 26.04; build from source if missing.
install_sdl3_mixer_from_source() {
    local version="3.2.4"
    local prefix="/usr/local"
    local srcdir
    srcdir="$(mktemp -d)"
    echo "📦 libsdl3-mixer-dev not in apt; building SDL3_mixer ${version} from source..."
    sudo apt install -y \
        libflac-dev \
        libmpg123-dev \
        libogg-dev \
        libopusfile-dev \
        libvorbis-dev
    curl -fsSL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${version}/SDL3_mixer-${version}.tar.gz" \
        | tar -xz -C "$srcdir"
    cmake -S "$srcdir/SDL3_mixer-${version}" -B "$srcdir/build" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -DSDLMIXER_VENDORED=OFF
    cmake --build "$srcdir/build" -j"$(nproc)"
    sudo cmake --install "$srcdir/build"
    rm -rf "$srcdir"
    export PKG_CONFIG_PATH="${prefix}/lib/pkgconfig:${prefix}/lib/x86_64-linux-gnu/pkgconfig:${prefix}/lib/aarch64-linux-gnu/pkgconfig:${PKG_CONFIG_PATH:-}"
    export LD_LIBRARY_PATH="${prefix}/lib:${prefix}/lib/x86_64-linux-gnu:${prefix}/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
}

case "$SDL_MODE" in
    sdl3)
        sudo apt install -y \
            libsdl3-dev \
            libsdl3-image-dev \
            libsdl3-ttf-dev
        if ! sudo apt install -y libsdl3-mixer-dev; then
            echo "⚠️ apt libsdl3-mixer-dev unavailable; falling back to source build"
            install_sdl3_mixer_from_source
        fi
        ;;
    sdl2)
        sudo apt install -y \
            libsdl2-dev \
            libsdl2-image-dev \
            libsdl2-ttf-dev \
            libsdl2-mixer-dev \
            libsdl2-gfx-dev
        ;;
    *)
        sudo apt install -y \
            libsdl1.2-dev \
            libsdl-image1.2-dev \
            libsdl-ttf2.0-dev \
            libsdl-mixer1.2-dev \
            libsdl-gfx1.2-dev
        ;;
esac

export QT_SELECT=qt5

echo "📁 Building rockbot the project..."
case "$SDL_MODE" in
    sdl3)
        SDL3_LIBS="$(pkg-config --libs sdl3 sdl3-image sdl3-ttf)"
        if pkg-config --exists sdl3-mixer; then
            SDL3_LIBS="$(pkg-config --libs sdl3 sdl3-image sdl3-ttf sdl3-mixer)"
        else
            SDL3_LIBS="${SDL3_LIBS} -lSDL3_mixer"
        fi
        qmake RockDroid.pro \
            CONFIG=linux \
            DESTDIR=build \
            DEFINES+=SDL3 \
            QMAKE_CCFLAGS+=-DSDL3 \
            QMAKE_CXXFLAGS+=-DSDL3 \
            "LIBS=${SDL3_LIBS} -ldl -lstdc++"
        ;;
    sdl2)
        qmake RockDroid.pro \
            CONFIG=linux \
            DESTDIR=build \
            DEFINES+=SDL2 \
            QMAKE_CCFLAGS+=-DSDL2 \
            QMAKE_CXXFLAGS+=-DSDL2 \
            LIBS="-lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx $(sdl2-config --libs)"
        ;;
    *)
        qmake RockDroid.pro CONFIG=linux DESTDIR=build
        ;;
esac
make clean build/rockbot

echo "📁 Building rockbot-editor the project..."
cd editor

case "$SDL_MODE" in
    sdl3)
        qmake Rockbot_Editor.pro \
            CONFIG+=linux \
            QMAKE_CCFLAGS+=-DSDL3 \
            QMAKE_CXXFLAGS+=-DSDL3 \
            DEFINES+=SDL3
        ;;
    sdl2)
        qmake Rockbot_Editor.pro \
            CONFIG+=linux \
            QMAKE_CCFLAGS+=-DSDL2 \
            QMAKE_CXXFLAGS+=-DSDL2 \
            DEFINES+=SDL2
        ;;
    *)
        qmake Rockbot_Editor.pro CONFIG+=linux
        ;;
esac

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
