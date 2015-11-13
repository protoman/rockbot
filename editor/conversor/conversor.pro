#-------------------------------------------------
#
# Project created by QtCreator 2011-11-20T10:58:05
#
#-------------------------------------------------

CONFIG -= linux # qtcreator adds linux even if shouldn't, so we remove
CONFIG -= app_bundle
CONFIG += console

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CCFLAGS += -fpermissive
QMAKE_CXXFLAGS += -fpermissive


TARGET = conversor

#CONFIG += win32
INCLUDEPATH += ..
INCLUDEPATH += ../..

win32 {
    LIBS = -lmingw32 -mwindows -lqtmaind -LC:\Qt\5.5\mingw492_32\lib

    INCLUDES = -I/usr/include/SDL \
    -I/usr/include \
    -I/include \
    -I. \
    -I./include \
    -L/usr/lib \
    -I.. \
    -I../..

    QMAKE_CCFLAGS += -DWIN32 -DPC
    QMAKE_CXXFLAGS += -DWIN32 -DPC -IC:\Qt\5.5\mingw492_32\ -IC:\Qt\5.5\mingw492_32\include -LC:\Qt\5.5\mingw492_32\lib -lmingw32 -mwindows -lqtmaind
}

TEMPLATE = app


SOURCES += main.cpp \
    fio_v1.cpp \
    ../../file/file_io.cpp \
    ../../file/fio_common.cpp \
    ../../aux_tools/stringutils.cpp

HEADERS += \
    v1.h \
    fio_v1.h \
    ../../file/file_io.h \
    ../../file/fio_common.h \
    ../../aux_tools/stringutils.h


