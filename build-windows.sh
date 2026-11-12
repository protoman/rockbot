#!/bin/bash
# Keep file/version.h equal to build/packages/version_name.txt
"$(dirname "${BASH_SOURCE[0]}")/build/packages/sync_version.sh"
set -e
set -o pipefail

# Cross-compile Rockbot for Windows from Linux (llvm-mingw, no root needed).
#
# Usage:
#   ./build-windows.sh                 → SDL2, x86_64 (default)
#   ./build-windows.sh sdl2            → SDL2, x86_64
#   ./build-windows.sh sdl3            → SDL3, x86_64
#   ./build-windows.sh sdl2 i686       → SDL2, 32-bit
#   ./build-windows.sh sdl2 x86_64 1   → also zip the Rockbot1 package
#   ./build-windows.sh sdl2 x86_64 2   → also zip the Rockbot2 package
#
# The Windows SDK lives next to the other platform SDKs:
#   $WINDOWS_SDK (default /mnt/ExtraData/Devel/SDK/Windows if /mnt/ExtraData/Devel/SDK exists,
#                 otherwise ~/.cache/rockbot-sdk/Windows)
#     llvm-mingw-<date>-ucrt-<host>/   cross toolchain (clang/LLD + MinGW-w64 UCRT);
#                                      downloaded automatically when missing
#     deps/mingw-<arch>-sdl<N>/        SDL, SDL_image, SDL_mixer, SDL_ttf, SDL_gfx
#                                      for Windows (downloaded/built on first use)
# Use another toolchain with MINGW_ROOT=/path/to/llvm-mingw (any tree that has
# bin/<triplet>-g++).
#
# Output:
#   build/rockbot_windows.exe
#   res/windows-dlls-<arch>-sdl<N>/*.dll   (also copied next to the exe)
#   build/packages/Rockbot<N>_Win64_<version>.zip (x86_64) or ..._Win32_... (i686), if game 1|2 given

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
            sed -n '7,29p' "$0" | sed 's/^# \{0,1\}//'
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

# Default SDK folder: the shared SDK disk when this machine has it, otherwise a folder in the home
# directory (CI runners and other machines cannot create /mnt/ExtraData).
if [[ -z "${WINDOWS_SDK:-}" ]]; then
    if [[ -d /mnt/ExtraData/Devel/SDK ]]; then
        WINDOWS_SDK="/mnt/ExtraData/Devel/SDK/Windows"
    else
        WINDOWS_SDK="${XDG_CACHE_HOME:-$HOME/.cache}/rockbot-sdk/Windows"
    fi
fi
LLVM_MINGW_VERSION="20261020"
HOST_ARCH="$(uname -m)"
case "$HOST_ARCH" in
    x86_64|amd64) LLVM_MINGW_HOST="ubuntu-22.04-x86_64" ;;
    aarch64|arm64) LLVM_MINGW_HOST="ubuntu-22.04-aarch64" ;;
    *) LLVM_MINGW_HOST="" ;;
esac

TRIPLET="${ARCH}-w64-mingw32"
DEPS_ROOT="${WINDOWS_SDK}/deps/mingw-${ARCH}-sdl${USE_SDL_VERSION}"
DEPS_PREFIX="${DEPS_ROOT}/prefix"
DOWNLOAD_DIR="${DEPS_ROOT}/downloads"
CMAKE_BUILD_DIR="${SCRIPT_DIR}/res/windows-${ARCH}-sdl${USE_SDL_VERSION}"
DLL_DIR="${SCRIPT_DIR}/res/windows-dlls-${ARCH}-sdl${USE_SDL_VERSION}"

# Pinned upstream releases (official *-mingw.tar.gz from libsdl-org).
SDL2_VERSION="2.32.8"
SDL2_IMAGE_VERSION="2.8.8"
SDL2_MIXER_VERSION="2.8.1"
SDL2_TTF_VERSION="2.24.0"
SDL2_GFX_VERSION="1.0.4"

SDL3_VERSION="3.4.18"
SDL3_IMAGE_VERSION="3.4.8"
SDL3_MIXER_VERSION="3.2.4"
SDL3_TTF_VERSION="3.2.2"

echo ">> Windows cross-build: SDL${USE_SDL_VERSION}, ${ARCH} (${TRIPLET})"

# --- host tools (no root: just report what is missing) -----------------------
missing=()
for tool in cmake make pkg-config curl tar rsync zip unzip; do
    command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
done
if [[ ${#missing[@]} -gt 0 ]]; then
    echo "Error: missing host tools: ${missing[*]}" >&2
    echo "       install them with your package manager and run again" >&2
    exit 1
fi

mkdir -p "$WINDOWS_SDK" "$DOWNLOAD_DIR" "$DEPS_PREFIX"

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

# --- cross toolchain ----------------------------------------------------------
if [[ -z "${MINGW_ROOT:-}" ]]; then
    MINGW_ROOT="$(ls -d "$WINDOWS_SDK"/llvm-mingw-*-ucrt-* 2>/dev/null | grep -v '\.tar' | sort -V | tail -1 || true)"
fi
if [[ -z "$MINGW_ROOT" || ! -x "$MINGW_ROOT/bin/${TRIPLET}-g++" ]]; then
    if [[ -z "$LLVM_MINGW_HOST" ]]; then
        echo "Error: no Windows toolchain in $WINDOWS_SDK and no prebuilt llvm-mingw for $HOST_ARCH." >&2
        echo "       Set MINGW_ROOT to a MinGW-w64 toolchain that has bin/${TRIPLET}-g++." >&2
        exit 1
    fi
    echo "🛠️ Installing llvm-mingw ${LLVM_MINGW_VERSION} into ${WINDOWS_SDK}..."
    archive="$WINDOWS_SDK/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-${LLVM_MINGW_HOST}.tar.xz"
    download "https://github.com/mstorsjo/llvm-mingw/releases/download/${LLVM_MINGW_VERSION}/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-${LLVM_MINGW_HOST}.tar.xz" "$archive"
    tar -xf "$archive" -C "$WINDOWS_SDK"
    MINGW_ROOT="$WINDOWS_SDK/llvm-mingw-${LLVM_MINGW_VERSION}-ucrt-${LLVM_MINGW_HOST}"
fi
if [[ ! -x "$MINGW_ROOT/bin/${TRIPLET}-g++" ]]; then
    echo "Error: ${TRIPLET}-g++ not found in $MINGW_ROOT/bin" >&2
    exit 1
fi
export MINGW_ROOT
export PATH="$MINGW_ROOT/bin:$PATH"
echo "🛠️ Toolchain: $MINGW_ROOT ($(${TRIPLET}-g++ --version | head -1))"

export MINGW_TARGET_TRIPLET="$TRIPLET"
export MINGW_SDL_PREFIX="$DEPS_PREFIX"

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

install_mixer_codecs() {
    # SDL_mixer loads its tracker-music codec (.mod/.s3m/.xm/.it) from a separate libxmp.dll,
    # which is only in the official *runtime* zip (optional/), not in the -devel package.
    local major="$1" version="$2" runtime_arch
    case "$ARCH" in
        i686) runtime_arch="win32-x86" ;;
        *)    runtime_arch="win32-x64" ;;
    esac
    local zip="$DOWNLOAD_DIR/SDL${major}_mixer-${version}-${runtime_arch}.zip"
    download "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${version}/SDL${major}_mixer-${version}-${runtime_arch}.zip" "$zip"
    mkdir -p "$DEPS_PREFIX/bin"
    unzip -o -j -q "$zip" 'optional/libxmp.dll' 'optional/LICENSE.xmp.txt' -d "$DEPS_PREFIX/bin"
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
    install_mixer_codecs 2 "$SDL2_MIXER_VERSION"
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
            export CC="${TRIPLET}-gcc" CXX="${TRIPLET}-g++" AR="${TRIPLET}-ar" RANLIB="${TRIPLET}-ranlib"
            ./configure \
                --host="$TRIPLET" \
                --prefix="$DEPS_PREFIX" \
                --with-sdl-prefix="$DEPS_PREFIX" \
                --disable-sdltest \
                --disable-mmx \
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
    install_mixer_codecs 3 "$SDL3_MIXER_VERSION"
    fix_pkgconfig_prefix
}

echo "📁 Preparing MinGW SDL${USE_SDL_VERSION} dependencies in ${DEPS_PREFIX}..."
if [[ ! -f "$DEPS_PREFIX/.rockbot_sdl${USE_SDL_VERSION}_ready_v2" ]]; then
    rm -rf "$DEPS_PREFIX"
    mkdir -p "$DEPS_PREFIX"
    case "$USE_SDL_VERSION" in
        3) install_sdl3_stack ;;
        *) install_sdl2_stack ;;
    esac
    touch "$DEPS_PREFIX/.rockbot_sdl${USE_SDL_VERSION}_ready_v2"
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

echo "📁 Building rockbot_windows.exe..."
cmake --build "$CMAKE_BUILD_DIR" -j"$(nproc)"

EXE_SRC="${SCRIPT_DIR}/build/rockbot_windows.exe"
if [[ ! -f "$EXE_SRC" ]]; then
    # Some generators put the binary under the cmake build dir.
    EXE_SRC="$(find "$CMAKE_BUILD_DIR" -name 'rockbot_windows.exe' | head -1)"
fi
if [[ -z "$EXE_SRC" || ! -f "$EXE_SRC" ]]; then
    echo "Error: rockbot_windows.exe not found after build"
    exit 1
fi

# Keep a stable path at build/rockbot_windows.exe
mkdir -p "${SCRIPT_DIR}/build"
if [[ "$EXE_SRC" != "${SCRIPT_DIR}/build/rockbot_windows.exe" ]]; then
    cp -f "$EXE_SRC" "${SCRIPT_DIR}/build/rockbot_windows.exe"
fi
EXE_SRC="${SCRIPT_DIR}/build/rockbot_windows.exe"

echo "📦 Collecting runtime DLLs..."
rm -rf "$DLL_DIR"
mkdir -p "$DLL_DIR"
# Every DLL the exe needs that is not part of Windows itself: found by following the
# import tables, looking in the SDL prefix and in the toolchain's runtime directory.
copy_dll_deps() {
    local binary="$1"
    local dll found
    while IFS= read -r dll; do
        [[ -f "$DLL_DIR/$dll" ]] && continue
        found=""
        for dir in "$DEPS_PREFIX/bin" "$MINGW_ROOT/${TRIPLET}/bin" "$MINGW_ROOT/bin" "/usr/${TRIPLET}/lib" "/usr/${TRIPLET}/bin"; do
            if [[ -f "$dir/$dll" ]]; then
                found="$dir/$dll"
                break
            fi
        done
        if [[ -n "$found" ]]; then
            cp -f "$found" "$DLL_DIR/$dll"
            copy_dll_deps "$DLL_DIR/$dll"
        fi
    done < <("${TRIPLET}-objdump" -p "$binary" 2>/dev/null | sed -n 's/^\s*DLL Name: //p')
}
copy_dll_deps "$EXE_SRC"
# SDL_image / SDL_mixer load their codec DLLs (png, jpeg, ogg, ...) at run time.
if [[ -d "$DEPS_PREFIX/bin" ]]; then
    for dll_path in "$DEPS_PREFIX"/bin/*.dll; do
        [[ -e "$dll_path" ]] || continue
        [[ -f "$DLL_DIR/$(basename "$dll_path")" ]] || cp -f "$dll_path" "$DLL_DIR"/
    done
fi

cp -f "$DEPS_PREFIX"/bin/LICENSE*.txt "$DLL_DIR"/ 2>/dev/null || true

# Place DLLs beside the exe for easy local testing / packaging
cp -f "$DLL_DIR"/*.dll "${SCRIPT_DIR}/build/" 2>/dev/null || true

"${TRIPLET}-strip" --strip-unneeded "$EXE_SRC" || true

echo "✅ Build completed successfully"
echo
echo "Binary:"
echo "  ${EXE_SRC}"
echo "DLLs:"
echo "  ${DLL_DIR}/  (also copied next to build/rockbot_windows.exe)"
echo
echo "Note: the Qt editor is not cross-compiled; build it natively on Windows if needed."

if [[ -n "$PACKAGE_VERSION" ]]; then
    VERSION_FILE="${SCRIPT_DIR}/build/packages/version_name.txt"
    GAME_DATA="${SCRIPT_DIR}/build/games/Rockbot${PACKAGE_VERSION}"
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
    case "$ARCH" in i686) WIN_TAG="Win32" ;; *) WIN_TAG="Win64" ;; esac
    OUT_ZIP="${SCRIPT_DIR}/build/packages/Rockbot${PACKAGE_VERSION}_${WIN_TAG}_${VERSION_NAME}.zip"

    echo "📦 Packaging ${WIN_TAG} zip for Rockbot${PACKAGE_VERSION} (${VERSION_NAME})..."
    rm -rf "$STAGE"
    mkdir -p "$STAGE/games"
    rsync -a --exclude='.svn' "${SCRIPT_DIR}/build/fonts" "$STAGE/"
    rsync -a --exclude='.svn' "${SCRIPT_DIR}/build/shared" "$STAGE/"
    rsync -a --exclude='.svn' "$GAME_DATA" "$STAGE/games/"
    cp -f "$EXE_SRC" "$STAGE/rockbot_windows.exe"
    cp -f "$DLL_DIR"/*.dll "$DLL_DIR"/LICENSE*.txt "$STAGE/" 2>/dev/null || true
    rm -f "$STAGE"/game*.sav "$STAGE"/config_v*.sav

    rm -f "$OUT_ZIP"
    (
        cd "$STAGE"
        zip -qr "$OUT_ZIP" .
    )
    echo "✅ Package: $OUT_ZIP"
fi

echo
echo "Usage on Windows:"
echo "  unzip the package and run rockbot_windows.exe (the game data folders must stay next to it)."
