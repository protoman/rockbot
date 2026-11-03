# Linux x64 build environment (tar.gz, deb, AppImage). Ubuntu 22.04 keeps the glibc
# requirement of the produced binaries low, so they run on most current distros.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates build-essential cmake make pkg-config git file zip fakeroot dpkg-dev \
        imagemagick ffmpeg wget xz-utils squashfs-tools desktop-file-utils zsync \
        libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev \
        libxmp-dev libx11-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /rockbot
