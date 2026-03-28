cmake_minimum_required(VERSION 3.10)
project(rockbot)

# Compiler Settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)


# Output Directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

# Add compiler flags from RockDroid.pro
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wno-reorder
        -Wno-ignored-qualifiers
        -fpermissive
        -Werror=return-type
        -fstack-protector-all
        -fstack-protector
    )
endif()

# Find Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL REQUIRED sdl)
pkg_check_modules(SDL_MIXER REQUIRED SDL_mixer)
pkg_check_modules(SDL_IMAGE REQUIRED SDL_image)
pkg_check_modules(SDL_TTF REQUIRED SDL_ttf)
pkg_check_modules(SDL_GFX REQUIRED SDL_gfx)
pkg_check_modules(X11 REQUIRED x11)

# Definitions
add_definitions(-DNES_RESOLUTION -DLINUX -DPC)

# Include Directories
include_directories(
    .
    ${SDL_INCLUDE_DIRS}
    ${SDL_MIXER_INCLUDE_DIRS}
    ${SDL_IMAGE_INCLUDE_DIRS}
    ${SDL_TTF_INCLUDE_DIRS}
    ${SDL_GFX_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Source Files
set(SOURCES
    main.cpp
    aux_tools/fps_control.cpp
    character/character.cpp
    data/datautil.cpp
    graphicslib.cpp
    inputlib.cpp
    game.cpp
    logger.cpp
    shareddata.cpp
    stage.cpp
    classmap.cpp
    character/classplayer.cpp
    timerlib.cpp
    sceneslib.cpp
    soundlib.cpp
    projectilelib.cpp
    character/classnpc.cpp
    stage_select.cpp
    class_config.cpp
    objects/object.cpp
    graphic/option_picker.cpp
    scenes/dialogs.cpp
    file/file_io.cpp
    graphic/animation.cpp
    character/artificial_inteligence.cpp
    options/key_map.cpp
    graphic/draw.cpp
    aux_tools/trajectory_parabola.cpp
    file/convert.cpp
    character/movement/jump.cpp
    character/movement/inertia.cpp
    file/fio_scenes.cpp
    scenes/sceneshow.cpp
    strings_map.cpp
    file/fio_strings.cpp
    aux_tools/stringutils.cpp
    file/fio_common.cpp
    game_mediator.cpp
    collision_detection.cpp
    graphic/gfx_sin_wave.cpp
    character/character_animation.cpp
    aux_tools/exception_manager.cpp
    scenes/game_menu.cpp
    ports/android/android_game_services.cpp
    sdl_layer.cpp
)

# Header Files
set(HEADERS
    aux_tools/fps_control.h
    character/character.h
    data/datautil.h
    file/v4/file_config_old.h
    file/v4/file_npc_extra_data.h
    graphicslib.h
    defines.h
    inputlib.h
    game.h
    logger.h
    shareddata.h
    stage.h
    classmap.h
    character/classplayer.h
    timerlib.h
    sceneslib.h
    soundlib.h
    projectilelib.h
    character/classnpc.h
    stage_select.h
    class_config.h
    objects/object.h
    graphic/option_picker.h
    scenes/dialogs.h
    file/format.h
    file/file_io.h
    graphic/animation.h
    character/artificial_inteligence.h
    ports/ps2/modules.h
    options/key_map.h
    graphic/draw.h
    aux_tools/trajectory_parabola.h
    file/convert.h
    file/format/st_characterState.h
    file/format/st_common.h
    file/format/st_hitPoints.h
    file/format/st_platform.h
    file/format/st_projectile.h
    file/format/st_teleporter.h
    character/movement/jump.h
    character/movement/inertia.h
    file/version.h
    file/fio_scenes.h
    scenes/sceneshow.h
    file/v4/file_config_v4.h
    file/v4/file_game_v4.h
    file/v4/file_save_v4.h
    file/v4/file_scene_v4.h
    file/v4/file_stage_v4.h
    strings_map.h
    file/fio_strings.h
    aux_tools/stringutils.h
    file/v4/file_strings.h
    file/v4/file_map.h
    file/fio_common.h
    game_mediator.h
    file/v4/file_anim_block.h
    collision_detection.h
    graphic/gfx_sin_wave.h
    character/character_animation.h
    aux_tools/exception_manager.h
    scenes/game_menu.h
    ports/android/android_game_services.h
    sdl_layer.h
)

add_executable(rockbot ${SOURCES} ${HEADERS})

# Link Libraries
target_link_libraries(rockbot
    ${SDL_LIBRARIES}
    ${SDL_MIXER_LIBRARIES}
    ${SDL_IMAGE_LIBRARIES}
    ${SDL_TTF_LIBRARIES}
    ${SDL_GFX_LIBRARIES}
    ${X11_LIBRARIES}
    dl
)
