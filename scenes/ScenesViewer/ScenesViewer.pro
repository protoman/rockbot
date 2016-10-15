#-------------------------------------------------
#
# Project created by QtCreator 2015-09-18T14:43:25
#
#-------------------------------------------------

CONFIG -= qt
CONFIG -= linux # qtcreator adds linux even if shouldn't, so we remove
QT       -= core
QT       -= gui


CONFIG += linux
#CONFIG += win32

TEMPLATE = app
INCLUDEPATH += ..
INCLUDEPATH += ../..

CONFIG += console
CONFIG -= app_bundle

TARGET = scenesviewer


linux {
        LIBS = -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf `sdl-config --libs`

        INCLUDES = -I/usr/include/SDL \
                -I/usr/include \
                -I. \
                -I.. \
                -I../include \
                -I../.. \
                -I../../include \
                -L/usr/lib
        QMAKE_CCFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers $${INCLUDES}
        QMAKE_CXXFLAGS += -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers $${INCLUDES}

}

win32 {
        LIBS =  -lSDL_mixer \
                -lSDL_image \
                -lSDL_ttf \
                -lmingw32 -lSDL -mwindows \
                -LC:\Qt\5.5\mingw492_32\lib


                INCLUDES = -I/usr/include/SDL \
                -I/usr/include \
                -I. \
                -I.. \
                -I../include \
                -I../.. \
                -I../../include \
                -L/usr/lib
                QMAKE_CCFLAGS += -DWIN32 -DPC $${INCLUDES}
                QMAKE_CXXFLAGS += -DWIN32 -DPC -IC:\Qt\5.5\mingw492_32\ -IC:\Qt\5.5\mingw492_32\include -LC:\Qt\5.5\mingw492_32\lib $${INCLUDES}
                #CONFIG -= console
}


SOURCES += main.cpp \
    ../../graphicslib.cpp \
    ../../inputlib.cpp \
    ../../soundlib.cpp \
    ../../timerlib.cpp \
    ../../file/file_io.cpp \
    ../../file/convert.cpp \
    ../sceneshow.cpp \
    ../../file/fio_scenes.cpp \
    ../../strings_map.cpp \
    ../../aux_tools/stringutils.cpp \
    ../../file/fio_strings.cpp \
    ../../game_mediator.cpp \
    ../../file/fio_common.cpp

HEADERS += \
    ../../graphicslib.h \
    ../../inputlib.h \
    ../../soundlib.h \
    ../../timerlib.h \
    ../../file/file_io.h \
    ../../file/convert.h \
    ../sceneshow.h \
    ../../file/v3/3_0_1/file_scene.h \
    ../../file/fio_scenes.h \
    ../../file/v4/file_config_v4.h \
    ../../file/v4/file_game_v4.h \
    ../../file/v4/file_save_v4.h \
    ../../file/v4/file_scene_v4.h \
    ../../file/v4/file_stage_v4.h \
    ../../file/v4/file_strings_v4.h \
    ../../file/v4/file_strings.h \
    ../../strings_map.h \
    ../../aux_tools/stringutils.h \
    ../../file/fio_strings.h \
    ../../game_mediator.h \
    ../../file/fio_common.h
