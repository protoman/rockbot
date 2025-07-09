#!/bin/bash
set -e

echo "🛠️ Installing dependencies with Homebrew..."
brew update
brew install \
    qt@5 \
    sdl2 \
    sdl2_image \
    sdl2_ttf \
    sdl2_mixer \
    sdl2_gfx

# Garantir que o qmake do Qt5 seja usado (ajuste conforme o caminho do brew)
export PATH="/opt/homebrew/opt/qt@5/bin:$PATH"
export QT_SELECT=qt5

echo "📁 Building rockbot the project..."
qmake RockDroid.pro CONFIG=macosx CONFIG+=sdk_no_version_check DESTDIR=build
make

echo "📁 Building rockbot-editor the project..."
(cd editor && \
qmake Rockbot_Editor.pro CONFIG=macosx && \
for f in $(find . -name '*.ui'); do \
  base=$(basename "$f" .ui); \
  echo "Generating ui_${base}.h from $f"; \
  uic "$f" -o "ui_${base}.h"; \
done && \
make)
echo "✅ Build completed successfully. Files are in ./build"

echo "Execute:"
echo "cd build && ./rockbot"
echo "./editor/editor"
