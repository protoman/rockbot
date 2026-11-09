cmake_minimum_required(VERSION 3.16)
project(RockbotEditor LANGUAGES C CXX)

# Rockbot level/data editor (Qt widgets). Replaces the old qmake project.
# Build:  cmake -S editor -B build/editor_cmake && cmake --build build/editor_cmake -j
# Builds with Qt5 and Qt6. Qt5 is the default (the Linux/macOS scripts use it); pass -DROCKBOT_EDITOR_QT=6 for Qt6.

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ROCKBOT_EDITOR_QT "" CACHE STRING "Qt major version to use (5 or 6); empty = auto")
if(ROCKBOT_EDITOR_QT STREQUAL "")
    set(ROCKBOT_EDITOR_QT 5)
endif()
find_package(Qt${ROCKBOT_EDITOR_QT} REQUIRED COMPONENTS Widgets)
message(STATUS "Rockbot editor: using Qt${ROCKBOT_EDITOR_QT}")

# SDL version define, as the qmake build passed (-DSDL2 / -DSDL3); 1 or empty adds nothing
set(USE_SDL_VERSION "" CACHE STRING "SDL major version the editor sources are built for (2 or 3)")
if(USE_SDL_VERSION STREQUAL "2" OR USE_SDL_VERSION STREQUAL "3")
    add_compile_definitions(SDL${USE_SDL_VERSION})
endif()

# the sources need -fpermissive (as in the qmake project)
add_compile_options(-fpermissive -g)
if(WIN32)
    add_compile_definitions(WIN32)
elseif(APPLE)
    add_compile_definitions(MACOSX)
endif()

set(EDITOR_SOURCES
    main.cpp
    ../shareddata.cpp
    ../strings_map.cpp
    mainwindow.cpp
    editorarea.cpp
    editortilepallete.cpp
    mediator.cpp
    addwizard.cpp
    loadgamepicker.cpp
    dialognpcedit.cpp
    dialogobjectedit.cpp
    npc_jump_preview_area.cpp
    projectilepreviewarea.cpp
    ../file/file_io.cpp
    player_preview_area.cpp
    mainwindow_tab/npc_edit.cpp
    common.cpp
    mainwindow_tab/object_tab.cpp
    mainwindow_tab/weapon_edit.cpp
    mainwindow_tab/stage_edit.cpp
    scenes/tab_parallax.cpp
    sprite_preview_area.cpp
    mainwindow_tab/artificial_inteligence_tab.cpp
    mainwindow_tab/projectile_edit.cpp
    mainwindow_tab/game_properties_tab.cpp
    mainwindow_tab/map_tab.cpp
    ../file/convert.cpp
    stage_swap_dialog.cpp
    mainwindow_tab/player_edit.cpp
    scenes/sceneeditorwindow.cpp
    scenes/tab_text.cpp
    mainwindow_tab/animtitle.cpp
    mainwindow_tab/armor_edit.cpp
    aboutwindow.cpp
    scenes/tab_image.cpp
    ../file/fio_scenes.cpp
    scenes/imageareapreview.cpp
    scenes/scenesmediator.cpp
    scenes/tab_viewpoint.cpp
    scenes/textpreviewarea.cpp
    scenes/tab_scenelist.cpp
    scenes/model/objectlistmodel.cpp
    scenes/comboboxdelegate.cpp
    scenes/tab_sfx.cpp
    scenes/tab_music.cpp
    scenes/tab_cleararea.cpp
    scenes/tab_animation.cpp
    scenes/animation_previewarea.cpp
    files_editor/fileseditor.cpp
    files_editor/image_directories_model.cpp
    model/combolistdelegate.cpp
    model/buttondelegate.cpp
    newgamedialog.cpp
    mainwindow_tab/gamescenes.cpp
    files_editor/stringseditor.cpp
    ../file/fio_strings.cpp
    ../aux_tools/stringutils.cpp
    model/stringeditmodel.cpp
    ../file/fio_common.cpp
    mainwindow_tab/anim_tiles_edit.cpp
    framespreviewarea.cpp
    widgets/animtilepalette.cpp
    mainwindow_tab/anim/animpackimport.cpp
    dialog_pick_color.cpp
    files_editor/gametextcreditstab.cpp
    ../aux_tools/exception_manager.cpp
    ../aux_tools/trajectory_parabola.cpp
    widgets/parallaxpreviewarea.cpp
)

set(EDITOR_HEADERS
    mainwindow.h
    ../shareddata.h
    ../strings_map.h
    editorarea.h
    editortilepallete.h
    mediator.h
    addwizard.h
    loadgamepicker.h
    dialognpcedit.h
    dialogobjectedit.h
    npc_jump_preview_area.h
    projectilepreviewarea.h
    ../file/file_io.h
    ../file/format.h
    player_preview_area.h
    mainwindow_tab/npc_edit.h
    common.h
    mainwindow_tab/object_tab.h
    mainwindow_tab/weapon_edit.h
    mainwindow_tab/stage_edit.h
    scenes/tab_parallax.h
    sprite_preview_area.h
    mainwindow_tab/artificial_inteligence_tab.h
    mainwindow_tab/projectile_edit.h
    enum_names.h
    mainwindow_tab/game_properties_tab.h
    mainwindow_tab/map_tab.h
    ../file/convert.h
    stage_swap_dialog.h
    mainwindow_tab/player_edit.h
    scenes/sceneeditorwindow.h
    scenes/tab_text.h
    mainwindow_tab/animtitle.h
    mainwindow_tab/armor_edit.h
    ../defines.h
    ../file/format/st_characterState.h
    ../file/format/st_common.h
    ../file/format/st_hitPoints.h
    ../file/format/st_platform.h
    ../file/format/st_projectile.h
    ../file/format/st_teleporter.h
    ../file/version.h
    aboutwindow.h
    scenes/tab_image.h
    scenes/imageareapreview.h
    scenes/scenesmediator.h
    scenes/tab_viewpoint.h
    scenes/textpreviewarea.h
    scenes/tab_scenelist.h
    scenes/model/objectlistmodel.h
    scenes/comboboxdelegate.h
    scenes/tab_sfx.h
    scenes/tab_music.h
    scenes/tab_cleararea.h
    scenes/tab_animation.h
    scenes/animation_previewarea.h
    files_editor/fileseditor.h
    files_editor/image_directories_model.h
    model/combolistdelegate.h
    model/buttondelegate.h
    newgamedialog.h
    ../file/v4/file_config_v4.h
    ../file/v4/file_game_v4.h
    ../file/v4/file_save_v4.h
    ../file/v4/file_scene_v4.h
    ../file/v4/file_stage_v4.h
    ../file/v4/file_strings.h
    mainwindow_tab/gamescenes.h
    files_editor/stringseditor.h
    ../file/fio_strings.h
    ../aux_tools/stringutils.h
    model/stringeditmodel.h
    ../file/fio_common.h
    mainwindow_tab/anim_tiles_edit.h
    framespreviewarea.h
    widgets/animtilepalette.h
    mainwindow_tab/anim/animpackimport.h
    dialog_pick_color.h
    files_editor/gametextcreditstab.h
    ../aux_tools/exception_manager.h
    ../aux_tools/trajectory_parabola.h
    widgets/parallaxpreviewarea.h
)

set(EDITOR_FORMS
    mainwindow.ui
    addwizard.ui
    loadgamepicker.ui
    dialognpcedit.ui
    dialogobjectedit.ui
    mainwindow_tab/npc_edit.ui
    mainwindow_tab/object_tab.ui
    mainwindow_tab/weapon_edit.ui
    mainwindow_tab/stage_edit.ui
    mainwindow_tab/artificial_inteligence_tab.ui
    mainwindow_tab/projectile_edit.ui
    mainwindow_tab/game_properties_tab.ui
    mainwindow_tab/map_tab.ui
    scenes/tab_parallax.ui
    stage_swap_dialog.ui
    mainwindow_tab/player_edit.ui
    scenes/sceneeditorwindow.ui
    scenes/tab_text.ui
    mainwindow_tab/armor_edit.ui
    aboutwindow.ui
    scenes/tab_image.ui
    scenes/tab_viewpoint.ui
    scenes/tab_scenelist.ui
    scenes/tab_sfx.ui
    scenes/tab_music.ui
    scenes/tab_cleararea.ui
    scenes/tab_animation.ui
    files_editor/fileseditor.ui
    newgamedialog.ui
    mainwindow_tab/gamescenes.ui
    files_editor/stringseditor.ui
    mainwindow_tab/anim_tiles_edit.ui
    mainwindow_tab/anim/animpackimport.ui
    dialog_pick_color.ui
    files_editor/gametextcreditstab.ui
)

# .ui files are listed so AUTOUIC picks them up
add_executable(rockbot_editor
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
    ${EDITOR_FORMS}
    resources/icons/icons.qrc
)
set_target_properties(rockbot_editor PROPERTIES
    OUTPUT_NAME editor
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build
)
if(WIN32)
    set_target_properties(rockbot_editor PROPERTIES WIN32_EXECUTABLE ON)
endif()

target_include_directories(rockbot_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow_tab
)
target_link_libraries(rockbot_editor PRIVATE Qt${ROCKBOT_EDITOR_QT}::Widgets)
if(APPLE)
    target_link_libraries(rockbot_editor PRIVATE z "-framework Carbon")
endif()
