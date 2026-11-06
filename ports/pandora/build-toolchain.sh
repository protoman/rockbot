#!/bin/bash
# Builds a current GCC cross compiler for the OpenPandora (ARMv7 Cortex-A8, softfp, glibc 2.9).
#
# The SDK shipped with the Pandora only has GCC 4.3.3 / Sourcery 4.6.1, too old for the game's C++11
# (no in-class member initializers before GCC 4.7). This builds binutils + GCC from source against the
# SDK sysroot, so libstdc++ is compiled against the device's own glibc 2.9 headers and the result
# runs on the stock firmware. Run it inside a throwaway container (see ports/pandora/README.md):
#
#   docker run --rm -v /mnt/ExtraData/Devel/SDK:/mnt/ExtraData/Devel/SDK -v $PWD/ports/pandora:/p \
#       ubuntu:26.04 bash /p/build-toolchain.sh
#
# Env: SDK_DIR (default /mnt/ExtraData/Devel/SDK), GCC_VER, BINUTILS_VER, NCPU.
set -euo pipefail

SDK_DIR="${SDK_DIR:-/mnt/ExtraData/Devel/SDK}"
GCC_VER="${GCC_VER:-16.2.0}"
BINUTILS_VER="${BINUTILS_VER:-2.47}"
GMP_VER=6.3.0; MPFR_VER=4.2.2; MPC_VER=1.3.1
TARGET=arm-angstrom-linux-gnueabi
SYSROOT="$SDK_DIR/OpenPandora/$TARGET"
PREFIX="$SDK_DIR/Pandora/gcc-$GCC_VER"
WORK="${WORK:-$SDK_DIR/Pandora/work}"   # kept after the build so a failed step can be resumed/inspected
JOBS="${NCPU:-$(nproc)}"

[ -f "$SYSROOT/usr/lib/libc.so" ] || { echo "OpenPandora sysroot not found at $SYSROOT" >&2; exit 1; }

export DEBIAN_FRONTEND=noninteractive
apt-get update -qq
apt-get install -y -qq build-essential curl xz-utils bzip2 file texinfo bison flex m4 >/dev/null

mkdir -p "$WORK" "$PREFIX"
cd "$WORK"
fetch() { [ -f "$(basename "$1")" ] || curl -fsSLO "$1"; }
fetch https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.xz
fetch https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.xz
fetch https://ftp.gnu.org/gnu/gmp/gmp-$GMP_VER.tar.xz
fetch https://ftp.gnu.org/gnu/mpfr/mpfr-$MPFR_VER.tar.xz
fetch https://ftp.gnu.org/gnu/mpc/mpc-$MPC_VER.tar.gz
for f in binutils-$BINUTILS_VER.tar.xz gcc-$GCC_VER.tar.xz gmp-$GMP_VER.tar.xz mpfr-$MPFR_VER.tar.xz mpc-$MPC_VER.tar.gz; do
    d="${f%.tar.*}"; [ -d "$d" ] || tar xf "$f"
done
# in-tree gmp/mpfr/mpc
for l in gmp-$GMP_VER:gmp mpfr-$MPFR_VER:mpfr mpc-$MPC_VER:mpc; do
    [ -e "gcc-$GCC_VER/${l#*:}" ] || ln -s "../${l%%:*}" "gcc-$GCC_VER/${l#*:}"
done

export PATH="$PREFIX/bin:$PATH"

# The glibc 2.9 crt*.o of the SDK carry Tag_ABI_HardFP_use "Deprecated" without an FP arch tag, which
# trips this (harmless) consistency assert in the ARM linker. Drop it.
sed -i 's|^\(\s*\)BFD_ASSERT (out_attr\[Tag_ABI_HardFP_use\].i == 0);|\1/* BFD_ASSERT (out_attr[Tag_ABI_HardFP_use].i == 0); (disabled for the Pandora SDK crt files) */|' \
    binutils-$BINUTILS_VER/bfd/elf32-arm.c
mkdir -p build-binutils && cd build-binutils
[ -f Makefile ] || ../binutils-$BINUTILS_VER/configure --target=$TARGET --prefix="$PREFIX" \
    --with-sysroot="$SYSROOT" --disable-nls --disable-werror --disable-gdb
[ -f .patched-ok ] || { make -j"$JOBS" && make install && touch .patched-ok; }
cd ..

mkdir -p build-gcc && cd build-gcc
[ -f Makefile ] || ../gcc-$GCC_VER/configure --target=$TARGET --prefix="$PREFIX" \
    --with-sysroot="$SYSROOT" --with-build-sysroot="$SYSROOT" \
    --enable-languages=c,c++ --disable-multilib --disable-nls --disable-bootstrap \
    --disable-libsanitizer --disable-libgomp --disable-libquadmath --disable-libssp --disable-libatomic \
    --disable-libitm --disable-libvtv --disable-libstdcxx-pch \
    --enable-threads=posix --enable-__cxa_atexit --enable-shared --enable-tls \
    --with-arch=armv7-a --with-tune=cortex-a8 --with-fpu=neon --with-float=softfp --with-mode=arm
make -j"$JOBS" all-gcc all-target-libgcc all-target-libstdc++-v3
make install-gcc install-target-libgcc install-target-libstdc++-v3
echo "Installed $PREFIX"
"$PREFIX/bin/$TARGET-g++" --version | head -1
