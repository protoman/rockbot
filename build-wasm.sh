#!/bin/bash

set -e

# Install Emscripten SDK:
# git clone https://github.com/emscripten-core/emsdk.git
# cd emsdk
# ./emsdk install latest
# ./emsdk activate latest
# source ./emsdk_env.sh
# Check with: emcc -v

# Compile all C++ files to WebAssembly (index.html output)
emcc -flto -O3 -fno-exceptions -fno-rtti \
main.cpp \
./shareddata.cpp \
./game.cpp \
./timerlib.cpp \
./data/datautil.cpp \
./strings_map.cpp \
./classmap.cpp \
./logger.cpp \
./options/key_map.cpp \
./stage_select.cpp \
./stage.cpp \
./character/character.cpp \
./character/movement/inertia.cpp \
./character/movement/jump.cpp \
./character/classplayer.cpp \
./character/character_animation.cpp \
./character/artificial_inteligence.cpp \
./character/classnpc.cpp \
./projectilelib.cpp \
./sceneslib.cpp \
./game_mediator.cpp \
./file/convert.cpp \
./file/fio_scenes.cpp \
./file/file_io.cpp \
./file/fio_common.cpp \
./file/fio_strings.cpp \
./objects/object.cpp \
./soundlib.cpp \
./inputlib.cpp \
./graphicslib.cpp \
./graphic/animation.cpp \
./graphic/draw.cpp \
./graphic/gfx_sin_wave.cpp \
./graphic/option_picker.cpp \
./collision_detection.cpp \
./class_config.cpp \
./scenes/dialogs.cpp \
./scenes/game_menu.cpp \
./scenes/sceneshow.cpp \
./aux_tools/stringutils.cpp \
./aux_tools/fps_control.cpp \
./aux_tools/exception_manager.cpp \
./aux_tools/trajectory_parabola.cpp \
./sdl_layer.cpp \
-I. \
-DSDL2 \
-sUSE_SDL=2 \
-sUSE_SDL_IMAGE=2 \
-sUSE_SDL_MIXER=2 \
-sUSE_SDL_TTF=2 \
-sUSE_SDL_GFX=2 \
-sSDL2_IMAGE_FORMATS='["png"]' \
-sSDL2_MIXER_FORMATS='["mod","wav"]' \
-sASSERTIONS \
-sASYNCIFY \
-sENVIRONMENT=web \
-sSTACK_SIZE=50mb \
-sTOTAL_MEMORY=128mb \
--preload-file build/fonts/@fonts/ --preload-file build/games/@games/ --preload-file build/shared/@shared/ \
--closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate','ALLOC_NORMAL'] \
-o index.html

# Replace index template
cp wasm.index.html index.html

echo "Serve files locally at http://localhost:8000"
echo "Execute the following command to start up the server"
echo "python -m http.server"