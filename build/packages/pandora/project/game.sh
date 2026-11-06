#!/bin/sh
# Launcher inside the .pnd (mounted by the Pandora menu); the game reads fonts/, shared/ and games/
# from its own folder and writes saves next to them (the pnd's appdata folder).
cd "$(dirname "$0")"
export LD_LIBRARY_PATH="$PWD/lib:$LD_LIBRARY_PATH"
export SDL_VIDEODRIVER=omapdss
export SDL_OMAP_LAYER_SIZE=800x480
export SDL_OMAP_VSYNC=1
export SDL_OMAP_FORCE_DOUBLEBUF=1
exec ./rockbot_pandora
