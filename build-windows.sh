#!/bin/bash
set -e
set -o pipefail

# Cross-compile Rockbot for Windows from Ubuntu (MinGW-w64).
#
# Usage:
#   ./build-windows.sh                 → SDL2, x86_64 (default)
#   ./build-windows.sh sdl2            → SDL2, x86_64
#   ./build-windows.sh sdl3            → SDL3, x86_64
#   ./build-windows.sh sdl2 i686       → SDL2, 32-bit
#   ./build-windows.sh sdl2 x86_64 1   → also zip RockDroid1 package
#   ./build-windows.sh sdl2 x86_64 2   → also zip RockDroid2 package
#
# Output:
#   build/rockbot.exe
#   build/windows-dlls/*.dll   (SDL + MinGW runtime DLLs next to the exe too)
#   build/packages/RockBot_Win32_<version>.zip  (if game version 1|2 given)

USE_SDL_VERSION=2
ARCH="x86_64"
PACKAGE_VERSION=""

for arg in "$@"; do
    case "$(printf '%s' "$arg" | tr '[:upper:]' '[:lower:]')" in
        sdl1)
            echo "Error: SDL1 Windows cross-build is not supported; use sdl2 or sdl3"
            exit 1
            ;;
        sdl2) USE_SDL_VERSION=2 ;;
        sdl3) USE_SDL_VERSION=3 ;;
        x86_64|amd64|win64) ARCH="x86_64" ;;
        i686|i386|win32|x86) ARCH="i686" ;;
        1|2) PACKAGE_VERSION="$arg" ;;
        -h|--help)
            echo "Usage: $0 [sdl2|sdl3] [x86_64|i686] [1|2]"
            echo
            echo "Cross-compiles rockbot.exe with MinGW-w64 on Ubuntu."
            echo "Optional 1|2 packs fonts/shared/games into a Win32 zip."
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [sdl2|sdl3] [x86_64|i686] [1|2]"
            exit 1
            ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

TRIPLET="${ARCH}-w64-mingw32"
DEPS_ROOT="${SCRIPT_DIR}/build/deps/mingw-${ARCH}-sdl${USE_SDL_VERSION}"
DEPS_PREFIX="${DEPS_ROOT}/prefix"
DOWNLOAD_DIR="${DEPS_ROOT}/downloads"
CMAKE_BUILD_DIR="${SCRIPT_DIR}/build/cmake-windows-${ARCH}-sdl${USE_SDL_VERSION}"
DLL_DIR="${SCRIPT_DIR}/build/windows-dlls-${ARCH}-sdl${USE_SDL_VERSION}"

# Pinned upstream releases (official *-mingw.tar.gz from libsdl-org).
SDL2_VERSION="2.32.8"
SDL2_IMAGE_VERSION="2.8.8"
SDL2_MIXER_VERSION="2.8.1"
SDL2_TTF_VERSION="2.24.0"
SDL2_GFX_VERSION="1.0.4"

SDL3_VERSION="3.2.16"
SDL3_IMAGE_VERSION="3.2.4"
SDL3_MIXER_VERSION="3.2.4"
SDL3_TTF_VERSION="3.2.2"

export DEBIAN_FRONTEND=noninteractive
export MINGW_TARGET_TRIPLET="$TRIPLET"
export MINGW_SDL_PREFIX="$DEPS_PREFIX"

echo ">> Windows cross-build: SDL${USE_SDL_VERSION}, ${ARCH} (${TRIPLET})"

echo "🛠️ Installing MinGW toolchain and build tools..."
sudo apt update
MINGW_PACKAGES=()
case "$ARCH" in
    x86_64) MINGW_PACKAGES+=(g++-mingw-w64-x86-64 gcc-mingw-w64-x86-64) ;;
    i686)   MINGW_PACKAGES+=(g++-mingw-w64-i686 gcc-mingw-w64-i686) ;;
esac
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    curl \
    ca-certificates \
    zip \
    unzip \
    tar \
    rsync \
    autoconf \
    automake \
    libtool \
    "${MINGW_PACKAGES[@]}"

if ! command -v "${TRIPLET}-g++" >/dev/null 2>&1; then
    echo "Error: ${TRIPLET}-g++ not found after apt install"
    exit 1
fi

mkdir -p "$DOWNLOAD_DIR" "$DEPS_PREFIX" "$DLL_DIR"

download() {
    local url="$1"
    local out="$2"
    if [[ -f "$out" ]]; then
        echo "📦 Cached: $(basename "$out")"
        return
    fi
    echo "⬇️  Downloading $(basename "$out")..."
    curl -fL --retry 3 --retry-delay 2 -o "$out.partial" "$url"
    mv "$out.partial" "$out"
}

extract_mingw_devel() {
    # Official SDL *-devel-*-mingw.tar.gz contains <triplet>/{bin,include,lib}.
    local archive="$1"
    local tmp
    tmp="$(mktemp -d)"
    tar -xzf "$archive" -C "$tmp"
    local arch_dir
    arch_dir="$(find "$tmp" -type d -name "$TRIPLET" | head -1)"
    if [[ -z "$arch_dir" || ! -d "$arch_dir" ]]; then
        echo "Error: $TRIPLET tree not found in $(basename "$archive")"
        rm -rf "$tmp"
        exit 1
    fi
    rsync -a "$arch_dir"/ "$DEPS_PREFIX"/
    rm -rf "$tmp"
}

fix_pkgconfig_prefix() {
    local pc_dir="$DEPS_PREFIX/lib/pkgconfig"
    [[ -d "$pc_dir" ]] || return 0
    # SDL mingw .pc files hardcode /opt or /usr/local — rewrite to our prefix.
    find "$pc_dir" -name '*.pc' -print0 | while IFS= read -r -d '' pc; do
        sed -i "s|^prefix=.*|prefix=${DEPS_PREFIX}|" "$pc"
    done
}

install_sdl2_stack() {
    download "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL2-devel-${SDL2_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-devel-${SDL2_MIXER_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL2_mixer-devel-${SDL2_MIXER_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.tar.gz"
    download "https://downloads.sourceforge.net/project/sdl2gfx/SDL2_gfx-${SDL2_GFX_VERSION}.tar.gz" \
        "$DOWNLOAD_DIR/SDL2_gfx-${SDL2_GFX_VERSION}.tar.gz"

    extract_mingw_devel "$DOWNLOAD_DIR/SDL2-devel-${SDL2_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL2_mixer-devel-${SDL2_MIXER_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL2_ttf-devel-${SDL2_TTF_VERSION}-mingw.tar.gz"
    fix_pkgconfig_prefix

    if [[ ! -f "$DEPS_PREFIX/lib/libSDL2_gfx.a" && ! -f "$DEPS_PREFIX/lib/libSDL2_gfx.dll.a" ]]; then
        echo "📦 Building SDL2_gfx ${SDL2_GFX_VERSION} for ${TRIPLET}..."
        local gfx_src
        gfx_src="$(mktemp -d)"
        tar -xzf "$DOWNLOAD_DIR/SDL2_gfx-${SDL2_GFX_VERSION}.tar.gz" -C "$gfx_src"
        (
            cd "$gfx_src/SDL2_gfx-${SDL2_GFX_VERSION}"
            # Older SDL2_gfx autoconf needs help finding cross SDL2.
            export PKG_CONFIG_LIBDIR="$DEPS_PREFIX/lib/pkgconfig"
            export PKG_CONFIG_PATH=""
            export PATH="$DEPS_PREFIX/bin:$PATH"
            ./configure \
                --host="$TRIPLET" \
                --prefix="$DEPS_PREFIX" \
                --with-sdl-prefix="$DEPS_PREFIX" \
                --disable-sdltest \
                SDL_CFLAGS="-I${DEPS_PREFIX}/include/SDL2" \
                SDL_LIBS="-L${DEPS_PREFIX}/lib -lSDL2"
            make -j"$(nproc)"
            make install
        )
        rm -rf "$gfx_src"
        fix_pkgconfig_prefix
    fi
}

install_sdl3_stack() {
    download "https://github.com/libsdl-org/SDL/releases/download/release-${SDL3_VERSION}/SDL3-devel-${SDL3_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL3-devel-${SDL3_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL3_IMAGE_VERSION}/SDL3_image-devel-${SDL3_IMAGE_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL3_image-devel-${SDL3_IMAGE_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL3_MIXER_VERSION}/SDL3_mixer-devel-${SDL3_MIXER_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL3_mixer-devel-${SDL3_MIXER_VERSION}-mingw.tar.gz"
    download "https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL3_TTF_VERSION}/SDL3_ttf-devel-${SDL3_TTF_VERSION}-mingw.tar.gz" \
        "$DOWNLOAD_DIR/SDL3_ttf-devel-${SDL3_TTF_VERSION}-mingw.tar.gz"

    extract_mingw_devel "$DOWNLOAD_DIR/SDL3-devel-${SDL3_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL3_image-devel-${SDL3_IMAGE_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL3_mixer-devel-${SDL3_MIXER_VERSION}-mingw.tar.gz"
    extract_mingw_devel "$DOWNLOAD_DIR/SDL3_ttf-devel-${SDL3_TTF_VERSION}-mingw.tar.gz"
    fix_pkgconfig_prefix
}

echo "📁 Preparing MinGW SDL${USE_SDL_VERSION} dependencies in ${DEPS_PREFIX}..."
if [[ ! -f "$DEPS_PREFIX/.rockbot_sdl${USE_SDL_VERSION}_ready" ]]; then
    rm -rf "$DEPS_PREFIX"
    mkdir -p "$DEPS_PREFIX"
    case "$USE_SDL_VERSION" in
        3) install_sdl3_stack ;;
        *) install_sdl2_stack ;;
    esac
    touch "$DEPS_PREFIX/.rockbot_sdl${USE_SDL_VERSION}_ready"
else
    echo "📦 Reusing existing SDL prefix: $DEPS_PREFIX"
fi

export PKG_CONFIG_LIBDIR="$DEPS_PREFIX/lib/pkgconfig"
export PKG_CONFIG_PATH=""

echo "📁 Configuring CMake (Windows / SDL${USE_SDL_VERSION})..."
rm -rf "$CMAKE_BUILD_DIR"
cmake -S "$SCRIPT_DIR" -B "$CMAKE_BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/mingw-w64-toolchain.cmake" \
    -DMINGW_TARGET_TRIPLET="$TRIPLET" \
    -DMINGW_SDL_PREFIX="$DEPS_PREFIX" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SDL_VERSION="${USE_SDL_VERSION}" \
    -DCMAKE_PREFIX_PATH="$DEPS_PREFIX"

echo "📁 Building rockbot.exe..."
cmake --build "$CMAKE_BUILD_DIR" -j"$(nproc)"

EXE_SRC="${SCRIPT_DIR}/build/rockbot.exe"
if [[ ! -f "$EXE_SRC" ]]; then
    # Some generators put the binary under the cmake build dir.
    EXE_SRC="$(find "$CMAKE_BUILD_DIR" -name 'rockbot.exe' | head -1)"
fi
if [[ -z "$EXE_SRC" || ! -f "$EXE_SRC" ]]; then
    echo "Error: rockbot.exe not found after build"
    exit 1
fi

# Keep a stable path at build/rockbot.exe
mkdir -p "${SCRIPT_DIR}/build"
if [[ "$EXE_SRC" != "${SCRIPT_DIR}/build/rockbot.exe" ]]; then
    cp -f "$EXE_SRC" "${SCRIPT_DIR}/build/rockbot.exe"
fi
EXE_SRC="${SCRIPT_DIR}/build/rockbot.exe"

echo "📦 Collecting runtime DLLs..."
rm -rf "$DLL_DIR"
mkdir -p "$DLL_DIR"
# SDL (and dependency) DLLs from the mingw prefix
if [[ -d "$DEPS_PREFIX/bin" ]]; then
    find "$DEPS_PREFIX/bin" -maxdepth 1 -name '*.dll' -exec cp -f {} "$DLL_DIR"/ \;
fi
# MinGW winpthread (still needed even with static libgcc/libstdc++)
for candidate in \
    "/usr/${TRIPLET}/lib/libwinpthread-1.dll" \
    "/usr/${TRIPLET}/bin/libwinpthread-1.dll" \
    "/usr/lib/gcc/${TRIPLET}/"*/libwinpthread-1.dll
do
    if [[ -f "$candidate" ]]; then
        cp -f "$candidate" "$DLL_DIR"/
        break
    fi
done

# Place DLLs beside the exe for easy local testing / packaging
cp -f "$DLL_DIR"/*.dll "${SCRIPT_DIR}/build/" 2>/dev/null || true

"${TRIPLET}-strip" --strip-unneeded "$EXE_SRC" || true

echo "✅ Build completed successfully"
echo
echo "Binary:"
echo "  ${EXE_SRC}"
echo "DLLs:"
echo "  ${DLL_DIR}/  (also copied next to build/rockbot.exe)"
echo
echo "Note: the Qt editor is not cross-compiled; build it natively on Windows if needed."

if [[ -n "$PACKAGE_VERSION" ]]; then
    VERSION_FILE="${SCRIPT_DIR}/build/packages/version_name_v${PACKAGE_VERSION}.txt"
    GAME_DATA="${SCRIPT_DIR}/build/games/RockDroid${PACKAGE_VERSION}"
    if [[ ! -f "$VERSION_FILE" ]]; then
        echo "Error: missing $VERSION_FILE"
        exit 1
    fi
    if [[ ! -d "$GAME_DATA" ]]; then
        echo "Error: missing game data: $GAME_DATA"
        exit 1
    fi
    VERSION_NAME="$(tr -d '[:space:]' < "$VERSION_FILE")"
    STAGE="${SCRIPT_DIR}/build/packages/win32/rockbot${PACKAGE_VERSION}"
    OUT_ZIP="${SCRIPT_DIR}/build/packages/RockBot_Win32_${VERSION_NAME}.zip"

    echo "📦 Packaging Win32 zip for RockDroid${PACKAGE_VERSION} (${VERSION_NAME})..."
    rm -rf "$STAGE"
    mkdir -p "$STAGE/games"
    rsync -a --exclude='.svn' "${SCRIPT_DIR}/build/fonts" "$STAGE/"
    rsync -a --exclude='.svn' "${SCRIPT_DIR}/build/shared" "$STAGE/"
    rsync -a --exclude='.svn' "$GAME_DATA" "$STAGE/games/"
    cp -f "$EXE_SRC" "$STAGE/rockbot.exe"
    cp -f "$DLL_DIR"/*.dll "$STAGE/" 2>/dev/null || true
    rm -f "$STAGE"/game*.sav "$STAGE"/config_v*.sav

    (
        cd "$STAGE"
        zip -qr "$OUT_ZIP" .
    )
    echo "✅ Package: $OUT_ZIP"
fi

echo
echo "Usage on Windows:"
echo "  1. Copy build/rockbot.exe + the DLLs into a folder with fonts/, shared/, games/"
echo "  2. Run rockbot.exe"
