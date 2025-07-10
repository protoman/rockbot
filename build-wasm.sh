#!/bin/bash

emcc main.cpp \
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
-I. \
-DSDL2 \
-sUSE_SDL=2 \
-sUSE_SDL_IMAGE=2 \
-sUSE_SDL_MIXER=2 \
-sUSE_SDL_TTF=2 \
-sUSE_SDL_GFX=2 \
-sSDL2_IMAGE_FORMATS='["png"]' \
-sFULL_ES3=1 \
-sASYNCIFY \
-sINITIAL_MEMORY=134217728 \
-sMAXIMUM_MEMORY=268435456 \
-sALLOW_MEMORY_GROWTH=1 \
--preload-file build@/ \
-o index.html

python3 -m http.server