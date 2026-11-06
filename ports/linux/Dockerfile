# Linux x64 build environment (tar.gz, deb, AppImage). Ubuntu 22.04 keeps the glibc
# requirement of the produced binaries low, so they run on most current distros.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates build-essential cmake make pkg-config git file zip fakeroot dpkg-dev \
        imagemagick ffmpeg wget curl rsync xz-utils squashfs-tools desktop-file-utils zsync \
        libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-gfx-dev \
        libxmp-dev libx11-dev \
 && rm -rf /var/lib/apt/lists/*
# appimagetool (latest release) baked in: the AppImage packaging script uses APPIMAGETOOL, so no download at build time
RUN mkdir -p /opt/appimagetool && curl -fL --retry 3 -o /opt/appimagetool/appimagetool-x86_64.AppImage \
        https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage \
 && chmod +x /opt/appimagetool/appimagetool-x86_64.AppImage
ENV APPIMAGETOOL=/opt/appimagetool/appimagetool-x86_64.AppImage
WORKDIR /rockbot
