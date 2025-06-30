#!/bin/bash
set -e

# Set non-interactive mode for apt
export DEBIAN_FRONTEND=noninteractive


echo "🛠️ Updating packages and installing dependencies..."
sudo apt update && sudo apt install -y \
    build-essential \
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    libgl1-mesa-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    libsdl2-mixer-dev \
    libsdl2-gfx-dev

export QT_SELECT=qt5

echo "📁 Building rockbot the project..."
qmake RockDroid.pro CONFIG=linux DESTDIR=build
make

echo "📁 Building rockbot-editor the project..."
(cd editor && \
qmake Rockbot_Editor.pro CONFIG+=linux && \
make)
echo "✅ Build completed successfully. Files are in ./build"


echo "Execute:"
echo "cd build && ./rockbot"
echo "GTK_PATH= ./editor"