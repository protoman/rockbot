#!/bin/sh

#if [ ! -d data ] ; then
#    cp -r data.files data
#fi

export SDL_VIDEODRIVER=omapdss
export SDL_OMAP_LAYER_SIZE=768x480
export SDL_OMAP_VSYNC=1
export SDL_OMAP_FORCE_DOUBLEBUF=1
LD_PRELOAD=./lib/libSDL-1.2.so.0.11.3 ./rockbot


