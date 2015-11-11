#-------------------------------------------------
#
# Project created by QtCreator 2011-11-20T10:58:05
#
#-------------------------------------------------

CONFIG -= qt
CONFIG -= linux # qtcreator adds linux even if shouldn't, so we remove
CONFIG   += console
CONFIG   -= app_bundle

QT       -= core
QT       -= gui


TARGET = conversor

CONFIG += win32
INCLUDEPATH += ..
INCLUDEPATH += ../..

win32 {
        LIBS =  -lmingw32 -lSDLmain -lSDL -mwindows -LC:\Qt\5.5\mingw492_32\lib

        INCLUDES = -I/usr/include/SDL \
        -I/usr/include \
        -I/include \
        -I. \
        -I./include \
        -L/usr/lib \
        -I.. \
        -I../..

        QMAKE_CCFLAGS += -DWIN32 -DPC
        QMAKE_CXXFLAGS += -DWIN32 -DPC -IC:\Qt\5.5\mingw492_32\ -IC:\Qt\5.5\mingw492_32\include -LC:\Qt\5.5\mingw492_32\lib
}

TEMPLATE = app


SOURCES += main.cpp \
    fio_v1.cpp

HEADERS += \
    v1.h \
    fio_v1.h


