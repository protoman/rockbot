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
#CONFIG += android

# -m64 problem in android build:
# NOTE for android build on 64: /usr/share/qt4/mkspecs/default/qmake.conf
# or, in QT5, /usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64/qmake.conf


### BETA VERSION ###
#DEFINESLIST += -DBETA_VERSION
#DEFINES += BETA_VERSION=1


# dolphin-emu -d -e ~/Desenvolvimento/rockbot/build/rockbot.elf


# sudo mount -o defaults,umask=000 ~/.local/share/dolphin-emu/Wii/sd.raw /media/sdcard
# cp -r ~/Desenvolvimento/rockbot/build/games/RockDroid2 /media/sdcard/RockDroid2
# cp -r ~/Desenvolvimento/rockbot/build/fonts /media/sdcard
# cp -r ~/Desenvolvimento/rockbot/build/shared /media/sdcard
# sudo umount /media/sdcard

CONFIG += console
CONFIG -= app_bundle
TARGET = rockdroid

linux {
        LIBS = -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `sdl-config --libs` -ldl

        INCLUDES = -I/usr/include/SDL \
                -I/usr/include \
                -I. \
                -I./include \
                -L/usr/lib
        QMAKE_CCFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive
        QMAKE_CXXFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive

}



android {
    DEFINES+= ANDROID=1
    ANDROIDSDK="/home/iuri/Programas/android-studio/sdk/"
    ANDROIDNDK="/home/iuri/Programas/android-studio/sdk/android-ndk-r14b"
    INCLUDEPATH += $${ANDROIDNDK}/platforms/android-14/arch-arm/usr/include


    TARGET = libapplication-armeabi.so

    QMAKE_CCFLAGS += -std=c++11 -pthread -DHAVE_PTHREADS             # C++ 11 is required by play services library
    QMAKE_CXXFLAGS += -std=c++11 -pthread -DHAVE_PTHREADS             # C++ 11 is required by play services library

    QMAKE_CXX = $${ANDROIDNDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++
    QMAKE_LINK = $${ANDROIDNDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++


    QMAKE_CXXFLAGS += -fpic -ffunction-sections -funwind-tables -fstack-protector \
    -DANDROID -DHANDHELD -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ \
    -D__ARM_ARCH_5TE__ -no-canonical-prefixes -march=armv5te -mtune=xscale -msoft-float -mthumb -fomit-frame-pointer \
    -fno-strict-aliasing -finline-limit=64 -DANDROID -DNDEBUG -O2 -finline-functions -Wa,--noexecstack \
    -D_GNU_SOURCE=1 -D_REENTRANT -shared -Wl,-soname,libapplication-armeabi.so \
    --sysroot=$${ANDROIDNDK}/platforms/android-14/arch-arm \
    -isystem$${ANDROIDNDK}/platforms/android-14/arch-arm/usr/include \
    -isystem$${ANDROIDNDK}/sources/cxx-stl/gnu-libstdc++/4.9/include \
    -isystem$${ANDROIDNDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl-1.2/include  \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/application/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl-1.2/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_main/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_image/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_mixer/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_ttf/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_gfx/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/stlport/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/jpeg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/png/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/ogg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/flac/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/vorbis/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/freetype/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/jpeg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/png/include \
    -I$${ANDROIDSDK}/rockbot_build/project/jni/sdl-1.2/include

    LIBS = -fpic -ffunction-sections -funwind-tables -fstack-protector \
    -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ \
    -D__ARM_ARCH_5TE__ -no-canonical-prefixes -march=armv5te -mtune=xscale -msoft-float -mthumb -fomit-frame-pointer \
    -fno-strict-aliasing -finline-limit=64 -DANDROID -DNDEBUG -O2 -finline-functions -Wa,--noexecstack \
    -D_GNU_SOURCE=1 -D_REENTRANT -shared -Wl,-soname,libapplication-armeabi.so \
    --sysroot=$${ANDROIDNDK}/platforms/android-14/arch-arm \
    -isystem$${ANDROIDNDK}/platforms/android-14/arch-arm/usr/lib \
    -isystem$${ANDROIDNDK}/platforms/android-14/arch-arm/usr/lib/c++/armeabi/ \
    -isystem$${ANDROIDNDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl-1.2/include  \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/application/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_main/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_image/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_mixer/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_ttf/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/sdl_gfx/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/stlport/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/jpeg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/png/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/ogg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/flac/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/vorbis/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/freetype/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/jpeg/include \
    -isystem$${ANDROIDSDK}/rockbot_build/project/jni/png/include \
    -I$${ANDROIDSDK}/rockbot_build/project/jni/sdl-1.2/include \
    --sysroot=$${ANDROIDNDK}/platforms/android-14/arch-arm \
    -L$${ANDROIDNDK}/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi \
    -L$${ANDROIDNDK}/platforms/android-14/arch-arm/usr/lib/ \
    -L$${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi \
    $${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi/libsdl-1.2.so \
    $${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi/libsdl_image.so \
    $${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi/libsdl_mixer.so \
    $${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi/libsdl_ttf.so \
    $${ANDROIDSDK}/rockbot_build/project/jni/../obj/local/armeabi/libsdl_gfx.so \
    -L$${ANDROIDSDK}/android-ndk-r14b/platforms/android-14/arch-arm/usr/lib \
    -lc -lm -lGLESv1_CM -ldl -llog -lz \
    -L/usr/lib \
    -L$${ANDROIDSDK}/android-ndk-r14b/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi \
    -lgnustl_static -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now \
    -lsupc++  -lsdl-1.2 \
    -llog -landroid -lEGL -lGLESv1_CM \
    -L$${ANDROIDSDK}/rockbot_build/ \
    #$${ANDROIDSDK}/android-ndk-r14b/platforms/android-14/arch-arm/usr/lib/c++/armeabi/libgpg.a # play services static library #


    #SOURCES +=
    #HEADERS +=

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



