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

linux {
        LIBS = -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf -lpng `sdl-config --libs`

        INCLUDES = -I/usr/include/SDL \
                -I/usr/include \
                -I. \
                -I.. \
                -I./include \
                -L/usr/lib
        QMAKE_CXXFLAGS += -DLINUX -DPC -Wno-reorder -O3 -Wno-ignored-qualifiers -I..

}


TEMPLATE = app


SOURCES += main.cpp \
    ../file/file_io.cpp \

HEADERS += \
    ../file/format.h \
    ../file/file_io.h \
    ../file/v_2_1_2.h \
    savepng.h




