#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e

# Usage:
#   ./build-slackware.sh        → SDL1.2 build
#   ./build-slackware.sh sdl2   → SDL2 build

if [[ "${1^^}" == "SDL2" ]]; then
    echo ">> SDL2 mode enabled"
    USE_SDL2=true
else
    USE_SDL2=false
fi

echo "🛠️ Checking for dependency tools..."

if ! command -v slackpkg &>/dev/null; then
    echo "⚠️ slackpkg not found. Package installation will be manual."
    HAVE_SLACKPKG=false
else
    HAVE_SLACKPKG=true
fi

echo "📦 Installing dependencies (if available)..."

install_pkg() {
    local pkg="$1"
    if $HAVE_SLACKPKG; then
        sudo slackpkg install "$pkg" || true
    else
        echo "👉 Please install manually: $pkg"
    fi
}

# Qt5 and build tools
install_pkg "qt5"
install_pkg "mesa"
install_pkg "gcc"
install_pkg "gcc-g++"
install_pkg "make"

# SDL deps
if $USE_SDL2; then
    install_pkg "SDL2"
    install_pkg "SDL2_image"
    install_pkg "SDL2_ttf"
    install_pkg "SDL2_mixer"
    install_pkg "SDL2_gfx"
else
    install_pkg "SDL"
    install_pkg "SDL_image"
    install_pkg "SDL_ttf"
    install_pkg "SDL_mixer"
    install_pkg "SDL_gfx"
fi

export QT_SELECT=qt5

echo "📁 Building rockbot the project..."

if $USE_SDL2; then
    qmake RockDroid.pro CONFIG=linux DESTDIR=build \
        DEFINES+=SDL2 \
        LIBS="-lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx $(sdl2-config --libs)"
else
    qmake RockDroid.pro CONFIG=linux DESTDIR=build
fi

make clean
make build/rockbot

echo "📁 Building rockbot-editor with CMake..."
SDLV=""
if $USE_SDL2; then SDLV=2; fi
cmake -S editor -B build/editor_cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SDL_VERSION="$SDLV"
cmake --build build/editor_cmake -j"$(nproc)"
echo "✅ Build completed successfully"

echo
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
echo
