# (Base is Ubuntu 26.04, newest LTS; the pspdev nightly needs glibc >= 2.38.)
# Console / handheld / Windows cross-build images. Each target bakes in its SDK from the
# host SDK folder (passed as the "sdk" build context) at the SAME absolute path the build
# scripts default to, so build-*.sh run unchanged inside the container.
#
#   build/docker/build_images.sh            builds every target below
#   docker build -f ports/docker/Dockerfile --target psp \
#       --build-context sdk=/mnt/ExtraData/Devel/SDK -t rockbot-psp ports/docker
ARG SDKROOT=/mnt/ExtraData/Devel/SDK

FROM ubuntu:26.04 AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates build-essential cmake make pkg-config git file zip unzip xz-utils bzip2 \
        python3 imagemagick ffmpeg wget curl rsync sed gawk patch \
        libgmp10 libmpfr6 libmpc3 libisl23 libzstd1 libssl3t64 zlib1g libncurses6 libexpat1 \
 && rm -rf /var/lib/apt/lists/*

FROM base AS psp
COPY --from=sdk PSP/pspdev-v20261101/pspdev /mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev

FROM base AS vita
COPY --from=sdk Vita/vitasdk /mnt/ExtraData/Devel/SDK/Vita/vitasdk

FROM base AS switch
COPY --from=sdk Switch/devkitpro /mnt/ExtraData/Devel/SDK/Switch/devkitpro

FROM base AS ps2
COPY --from=sdk PS2.new/ps2dev /mnt/ExtraData/Devel/SDK/PS2.new/ps2dev

FROM base AS ps3
COPY --from=sdk PS3/ps3dev /mnt/ExtraData/Devel/SDK/PS3/ps3dev

FROM base AS wii
COPY --from=sdk Wii/devkitpro-current /mnt/ExtraData/Devel/SDK/Wii/devkitpro-current

FROM base AS windows
COPY --from=sdk Windows/deps /mnt/ExtraData/Devel/SDK/Windows/deps
COPY --from=sdk Windows/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64 /mnt/ExtraData/Devel/SDK/Windows/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64

FROM base AS dingux
# the legacy A320 toolchain (GCC 4.7.1) is made of 32-bit x86 programs
RUN dpkg --add-architecture i386 && apt-get update && apt-get install -y --no-install-recommends libc6:i386 libstdc++6:i386 zlib1g:i386 && rm -rf /var/lib/apt/lists/*
COPY --from=sdk opendingux /mnt/ExtraData/Devel/SDK/opendingux

FROM base AS pandora
# GCC built by ports/pandora/build-toolchain.sh against the OpenPandora sysroot; mksquashfs makes the .pnd
RUN apt-get update && apt-get install -y --no-install-recommends squashfs-tools && rm -rf /var/lib/apt/lists/*
COPY --from=sdk OpenPandora/arm-angstrom-linux-gnueabi /mnt/ExtraData/Devel/SDK/OpenPandora/arm-angstrom-linux-gnueabi
COPY --from=sdk Pandora/gcc-16.2.0 /mnt/ExtraData/Devel/SDK/Pandora/gcc-16.2.0

FROM base AS pocketgo
COPY --from=sdk PocketGo/output/host /mnt/ExtraData/Devel/SDK/PocketGo/output/host
# the Buildroot host tools carry the RUNPATH of the machine that built them
ENV LD_LIBRARY_PATH=/mnt/ExtraData/Devel/SDK/PocketGo/output/host/lib
