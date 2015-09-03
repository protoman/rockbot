#-------------------------------------------------
#
# Project created by QtCreator 2015-06-27T08:23:45
#
#-------------------------------------------------

QT       += core gui

CONFIG += linux
#CONFIG += win32


win32 {
    QMAKE_CCFLAGS += -DWIN32
    QMAKE_CXXFLAGS += -DWIN32
}

linux {
    QMAKE_CCFLAGS += -DLINUX
    QMAKE_CXXFLAGS += -DLINUX
}


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PasswordGenerator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../scenes/password_generator.cpp \
    ../../file/file_io.cpp \
    ../../file/convert.cpp \
    passwordgrid.cpp

HEADERS  += mainwindow.h \
    ../../scenes/password_generator.h \
    ../../defines.h \
    ../../file/file_io.h \
    ../../file/convert.h \
    passwordgrid.h

FORMS    += mainwindow.ui
