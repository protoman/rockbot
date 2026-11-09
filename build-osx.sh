#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Usage:
#   ./build-osx.sh          → SDL2 (default)
#   ./build-osx.sh sdl2     → SDL2
#   ./build-osx.sh sdl3     → SDL3

MODE="$(printf '%s' "${1:-sdl2}" | tr '[:upper:]' '[:lower:]')"
case "$MODE" in
    sdl2)
        USE_SDL_VERSION=2
        echo ">> SDL2 mode enabled"
        ;;
    sdl3)
        USE_SDL_VERSION=3
        echo ">> SDL3 mode enabled"
        ;;
    *)
        echo "Usage: $0 [sdl2|sdl3]"
        exit 1
        ;;
esac

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "🛠️ Installing dependencies with Homebrew..."
brew update
brew install \
    cmake \
    pkg-config \
    qt@5

if [ "$USE_SDL_VERSION" -eq 3 ]; then
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

# Homebrew pkg-config files (Apple Silicon and Intel)
BREW_PREFIX="$(brew --prefix)"
QT5_PREFIX="$(brew --prefix qt@5 2>/dev/null || true)"
if [ ! -d "${QT5_PREFIX}/bin" ]; then
    for candidate in "${BREW_PREFIX}/opt/qt@5" /opt/homebrew/opt/qt@5 /usr/local/opt/qt@5; do
        if [ -d "${candidate}/bin" ]; then
            QT5_PREFIX="$candidate"
            break
        fi
    done
fi
if [ ! -x "${QT5_PREFIX}/bin/qmake" ]; then
    echo "❌ Qt5 qmake not found. Install with: brew install qt@5" >&2
    exit 1
fi
export PATH="${QT5_PREFIX}/bin:${PATH}"
export PKG_CONFIG_PATH="${BREW_PREFIX}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
export QT_SELECT=qt5

# Resolve the macOS SDK actually installed (e.g. macosx26.2).
MAC_SDK_VERSION="$(xcrun --sdk macosx --show-sdk-version)"
MAC_SDK="macosx${MAC_SDK_VERSION}"
echo "📦 Using macOS SDK: ${MAC_SDK}"

CMAKE_BUILD_DIR="${SCRIPT_DIR}/res/cmake"
echo "📁 Building rockbot with CMake (SDL${USE_SDL_VERSION})..."
rm -rf "$CMAKE_BUILD_DIR"
cmake -S "$SCRIPT_DIR" -B "$CMAKE_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL_VERSION="${USE_SDL_VERSION}"

NPROC="$(sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
cmake --build "$CMAKE_BUILD_DIR" -j"$NPROC"

echo "📁 Building rockbot-editor with CMake..."
# Qt5 from Homebrew (QT5_PREFIX) (the editor code is not Qt6-ready); output is build/editor (no .app bundle)
rm -rf "${SCRIPT_DIR}/build/editor" "${SCRIPT_DIR}/build/editor.app"
EDITOR_BUILD_DIR="${SCRIPT_DIR}/build/editor_cmake"
cmake -S "${SCRIPT_DIR}/editor" -B "$EDITOR_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="${QT5_PREFIX}" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.7 \
    -DCMAKE_OSX_SYSROOT="${MAC_SDK}" \
    -DUSE_SDL_VERSION="${USE_SDL_VERSION}"
cmake --build "$EDITOR_BUILD_DIR" -j"$NPROC"

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
