# -------------------------------------------------
# Project created by QtCreator 2009-12-03T12:16:29
# -------------------------------------------------

#CONFIG += win32
CONFIG += linux
#CONFIG += macosx

QT += widgets

macosx {
        QMAKE_LIBS_QT =
        QMAKE_LIBS_QT_THREAD =

    QMAKE_CXX = /usr/local/bin/g++-4.8
    QMAKE_LINK = /usr/local/bin/g++-4.8
    QMAKE_CXXFLAGS += -I/usr/local/Trolltech/Qt-4.8.6/include -I/usr/local/Trolltech/Qt-4.8.6/include/QtGui -DMACOSX


    QT = core gui
    LIBS += -static-libgcc -static-libstdc++ /usr/local/Trolltech/Qt-4.8.6/lib/libQtCore.a /usr/local/Trolltech/Qt-4.8.6/lib/libQtGui.a -lz -framework Carbon
    QMAKE_LFLAGS -= -framework QtGui
    QMAKE_CFLAGS_PPC_64             -= -arch ppc64 -Xarch_ppc64 -mmacosx-version-min=10.5
    QMAKE_OBJECTIVE_CFLAGS_PPC_64   -= -arch ppc64 -Xarch_ppc64 -mmacosx-version-min=10.5
    QMAKE_CFLAGS_X86_64             -= -Xarch_x86_64 -mmacosx-version-min=10.5
    QMAKE_OBJECTIVE_CFLAGS_X86_64   -= -Xarch_x86_64 -mmacosx-version-min=10.5
    QMAKE_CXXFLAGS_PPC_64           -= -arch ppc64 -Xarch_ppc64 -mmacosx-version-min=10.5
    QMAKE_CXXFLAGS_X86_64           -= -Xarch_x86_64 -mmacosx-version-min=10.5
    QMAKE_LFLAGS_PPC_64             -= -arch ppc64 -Xarch_ppc64 -mmacosx-version-min=10.5
    QMAKE_LFLAGS_X86_64             -= -Xarch_x86_64 -mmacosx-version-min=10.5
}

linux {
    QMAKE_CCFLAGS += -std=c++0x
    QMAKE_CXXFLAGS += -std=c++0x
}

win32 {
    QMAKE_CCFLAGS += -std=c++0x
    QMAKE_CXXFLAGS += -std=c++0x
    CXXFLAGS += -std=c++0x
    LIBS = -lmingw32 -mwindows -lqtmaind
    QT += core gui
    QMAKE_CCFLAGS += -DWIN32
    QMAKE_CXXFLAGS += -DWIN32
    CXXFLAGS += -DWIN32
}


TARGET = ../build/editor
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    editorarea.cpp \
    editortilepallete.cpp \
    mediator.cpp \
    addwizard.cpp \
    npcpreviewarea.cpp \
    loadgamepicker.cpp \
    dialognpcedit.cpp \
    dialogobjectedit.cpp \
    projectilepreviewarea.cpp \
    ../file/file_io.cpp \
    dialog_pick_color.cpp \
	player_preview_area.cpp \
    mainwindow_tab/npc_edit.cpp \
    common.cpp \
    mainwindow_tab/object_tab.cpp \
    mainwindow_tab/weapon_edit.cpp \
    mainwindow_tab/stage_edit.cpp \
    sprite_preview_area.cpp \
    mainwindow_tab/artificial_inteligence_tab.cpp \
    mainwindow_tab/projectile_edit.cpp \
    mainwindow_tab/colorcycle.cpp \
    mainwindow_tab/colorcycle_map_preview.cpp \
    mainwindow_tab/game_properties_tab.cpp \
    mainwindow_tab/map_tab.cpp \
    ../file/convert.cpp \
    stage_swap_dialog.cpp \
    mainwindow_tab/player_edit.cpp \
    scenes/sceneeditorwindow.cpp \
    scenes/sceneslist.cpp \
    scenes/tab_text.cpp \
    mainwindow_tab/animtitle.cpp \
    mainwindow_tab/armor_edit.cpp \
    aboutwindow.cpp \
    ../tools/tinyxml2/tinyxml2.cpp \
    scenes/tab_image.cpp \
    ../file/fio_scenes.cpp

HEADERS += mainwindow.h \
    editorarea.h \
    editortilepallete.h \
    mediator.h \
    addwizard.h \
    addwizard.h \
    npcpreviewarea.h \
    loadgamepicker.h \
    dialognpcedit.h \
    dialogobjectedit.h \
    projectilepreviewarea.h \
    ../file/file_io.h \
    ../file/format.h \
    dialog_pick_color.h \
	player_preview_area.h \
    mainwindow_tab/npc_edit.h \
    common.h \
    mainwindow_tab/object_tab.h \
    mainwindow_tab/weapon_edit.h \
    mainwindow_tab/stage_edit.h \
    sprite_preview_area.h \
    mainwindow_tab/artificial_inteligence_tab.h \
    mainwindow_tab/projectile_edit.h \
    enum_names.h \
    mainwindow_tab/colorcycle.h \
    mainwindow_tab/colorcycle_map_preview.h \
    mainwindow_tab/game_properties_tab.h \
    mainwindow_tab/map_tab.h \
    ../file/convert.h \
    stage_swap_dialog.h \
    mainwindow_tab/player_edit.h \
    scenes/sceneeditorwindow.h \
    scenes/sceneslist.h \
    scenes/tab_text.h \
    mainwindow_tab/animtitle.h \
    mainwindow_tab/armor_edit.h \
    ../defines.h \
    ../file/format/st_characterState.h \
    ../file/format/st_common.h \
    ../file/format/st_hitPoints.h \
    ../file/format/st_platform.h \
    ../file/format/st_projectile.h \
    ../file/format/st_teleporter.h \
    ../file/version.h \
    aboutwindow.h \
    ../tools/tinyxml2/tinyxml2.h \
    scenes/tab_image.h

FORMS += mainwindow.ui \
    addwizard.ui \
    loadgamepicker.ui \
    dialognpcedit.ui \
    dialogobjectedit.ui \
	dialog_pick_color.ui \
    mainwindow_tab/npc_edit.ui \
    mainwindow_tab/object_tab.ui \
    mainwindow_tab/weapon_edit.ui \
    mainwindow_tab/stage_edit.ui \
    mainwindow_tab/artificial_inteligence_tab.ui \
    mainwindow_tab/projectile_edit.ui \
    mainwindow_tab/colorcycle.ui \
    mainwindow_tab/game_properties_tab.ui \
    mainwindow_tab/map_tab.ui \
    stage_swap_dialog.ui \
    mainwindow_tab/player_edit.ui \
    scenes/sceneeditorwindow.ui \
    scenes/sceneslist.ui \
    scenes/tab_text.ui \
    mainwindow_tab/armor_edit.ui \
    aboutwindow.ui \
    scenes/tab_image.ui
RESOURCES += resources/icons/icons.qrc
CONFIG += console
INCLUDEPATH += ../common
INCLUDEPATH += ..
INCLUDEPATH += ./mainwindow_tab
OTHER_FILES += ../docs/RoadMap \
    ../docs/ChangeLog.txt






















