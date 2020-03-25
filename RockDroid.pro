#-------------------------------------------------
#
# Project created by QtCreator 2011-03-05T22:53:17
#
#-------------------------------------------------

CONFIG -= qt
CONFIG -= linux # qtcreator adds linux even if shouldn't, so we remove
QT       -= core
QT       -= gui



CONFIG += linux
#CONFIG += win32

#DEFINES+= ANDROID=1

# -m64 problem in android build:
# NOTE for android build on 64: /usr/share/qt4/mkspecs/default/qmake.conf
# or, in QT5, /usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64/qmake.conf


### BETA VERSION ###
#DEFINESLIST += -DBETA_VERSION
#DEFINES += BETA_VERSION=1


CONFIG += console
CONFIG -= app_bundle
TARGET = rockbot

linux {
    LIBS = -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `sdl-config --libs` -ldl -lstdc++
    INCLUDES = -I/usr/include/SDL -I/usr/include -I. -I./include -L/usr/lib
    QMAKE_CCFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive -mtune=generic -Werror=return-type
    QMAKE_CXXFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive -mtune=generic -Werror=return-type
}

win32 {
    LIBS =  -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx -lmingw32 -lSDL -mwindows -LC:\Qt\4.8.4\lib
    INCLUDES = -I/usr/include/SDL -I/usr/include -I. -I./include -L/usr/lib
    QMAKE_CCFLAGS += -DWIN32 -DPC
    QMAKE_CXXFLAGS += -DWIN32 -DPC -IC:\Qt\4.8.4\ -IC:\Qt\4.8.4\include -LC:\Qt\4.8.4\lib
    CONFIG -= console
}



QMAKE_CXXFLAGS += $${DEFINESLIST}

TEMPLATE = app


SOURCES += main.cpp \
    aux_tools/fps_control.cpp \
    character/character.cpp \
    graphicslib.cpp \
    inputlib.cpp \
    game.cpp \
    stage.cpp \
    classmap.cpp \
    character/classplayer.cpp \
    timerlib.cpp \
    sceneslib.cpp \
    soundlib.cpp \
    projectilelib.cpp \
    character/classnpc.cpp \
    stage_select.cpp \
    class_config.cpp \
    objects/object.cpp \
    graphic/option_picker.cpp \
    scenes/dialogs.cpp \
    file/file_io.cpp \
    graphic/animation.cpp \
    character/artificial_inteligence.cpp \
    options/key_map.cpp \
    graphic/draw.cpp \
    aux_tools/trajectory_parabola.cpp \
    file/convert.cpp \
    character/movement/jump.cpp \
    character/movement/inertia.cpp \
    file/fio_scenes.cpp \
    scenes/sceneshow.cpp \
    strings_map.cpp \
    file/fio_strings.cpp \
    aux_tools/stringutils.cpp \
    file/fio_common.cpp \
    game_mediator.cpp \
    collision_detection.cpp \
    graphic/gfx_sin_wave.cpp \
    character/character_animation.cpp \
    aux_tools/exception_manager.cpp \
    scenes/game_menu.cpp \
    ports/android/android_game_services.cpp


HEADERS += \
    aux_tools/fps_control.h \
    character/character.h \
    graphicslib.h \
    defines.h \
    inputlib.h \
    game.h \
    stage.h \
    classmap.h \
    character/classplayer.h \
    timerlib.h \
    sceneslib.h \
    soundlib.h \
    projectilelib.h \
    character/classnpc.h \
    stage_select.h \
    class_config.h \
    objects/object.h \
    graphic/option_picker.h \
    scenes/dialogs.h \
    file/format.h \
    file/file_io.h \
    graphic/animation.h \
    character/artificial_inteligence.h \
    character/st_spriteFrame.h \
    ports/ps2/modules.h \
    options/key_map.h \
    graphic/draw.h \
    aux_tools/trajectory_parabola.h \
    file/convert.h \
    file/format/st_characterState.h \
    file/format/st_common.h \
    file/format/st_hitPoints.h \
    file/format/st_platform.h \
    file/format/st_projectile.h \
    file/format/st_teleporter.h \
    character/movement/jump.h \
    character/movement/inertia.h \
    file/version.h \
    file/fio_scenes.h \
    scenes/sceneshow.h \
    file/v4/file_config_v4.h \
    file/v4/file_game_v4.h \
    file/v4/file_save_v4.h \
    file/v4/file_scene_v4.h \
    file/v4/file_stage_v4.h \
    strings_map.h \
    file/fio_strings.h \
    aux_tools/stringutils.h \
    file/v4/file_strings.h \
    file/v4/file_map.h \
    file/fio_common.h \
    game_mediator.h \
    file/v4/file_anim_block.h \
    collision_detection.h \
    graphic/gfx_sin_wave.h \
    character/character_animation.h \
    aux_tools/exception_manager.h \
    scenes/game_menu.h \
    file/v4/file_castlepoints.h \
    ports/android/android_game_services.h

OTHER_FILES += \
    docs/RoadMap.txt \
    rockbot_dingux.pro \
    docs/Changelog.txt \
    docs/graphic_conversion.txt

DISTFILES += \
    docs/notes.txt \
    docs/Releases.txt \
    docs/crashes.txt \
    docs/translation_lines.txt



