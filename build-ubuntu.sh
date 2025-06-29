#!/bin/bash
set -e

# Set non-interactive mode for apt
export DEBIAN_FRONTEND=noninteractive

echo "🛠️ Updating packages and installing dependencies..."
sudo apt update && sudo apt install -y \
    build-essential \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    libsdl1.2-dev \
    libsdl-image1.2-dev \
    libsdl-ttf2.0-dev \
    libsdl-mixer1.2-dev \
    libsdl-gfx1.2-dev

# Set Qt6 as default
export QT_SELECT=qt6

# Assume we are already in the project directory
echo "📁 Building the project..."
qmake6 RockDroid.pro
make

echo "📦 Copying files to build directory..."
pkill -9 rockbot && sleep 1 && cp rockbot build

echo "✅ Build completed successfully. Files are in ./build"

echo "Execute to run: cd build && ./rockbot"
