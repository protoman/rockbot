# Cross toolchains and ARM SDL2 development libraries for Raspberry Pi OS (Debian bookworm):
# armhf = 32-bit Raspberry Pi OS (Pi 2/3/4/5, Zero 2), arm64 = 64-bit Raspberry Pi OS (Pi 3/4/5).
FROM debian:bookworm-slim
ENV DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture armhf && dpkg --add-architecture arm64 \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
        ca-certificates cmake make pkg-config \
        crossbuild-essential-armhf crossbuild-essential-arm64 \
        libsdl2-dev:armhf libsdl2-image-dev:armhf libsdl2-mixer-dev:armhf libsdl2-ttf-dev:armhf libsdl2-gfx-dev:armhf libx11-dev:armhf \
        libsdl2-dev:arm64 libsdl2-image-dev:arm64 libsdl2-mixer-dev:arm64 libsdl2-ttf-dev:arm64 libsdl2-gfx-dev:arm64 libx11-dev:arm64 \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /rockbot
