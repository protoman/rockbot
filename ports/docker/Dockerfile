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
        python3 imagemagick ffmpeg wget curl sed gawk patch \
        libgmp10 libmpfr6 libmpc3 libisl23 libzstd1 libssl3t64 zlib1g libncurses6 libexpat1 \
 && rm -rf /var/lib/apt/lists/*

FROM base AS psp
COPY --from=sdk PSP/pspdev-v20261101/pspdev /mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev

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
COPY --from=sdk opendingux-gcw0-2022-09-22/gcw0-toolchain /mnt/ExtraData/Devel/SDK/opendingux-gcw0-2022-09-22/gcw0-toolchain

FROM base AS pocketgo
COPY --from=sdk PocketGo/output/host /mnt/ExtraData/Devel/SDK/PocketGo/output/host
