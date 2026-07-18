cmake_minimum_required(VERSION 3.10)
project(rockbot)

# Compiler Settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output Directory (same location as DESTDIR=build used by the old qmake scripts)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

# SDL Version Selection (override with: cmake -DUSE_SDL_VERSION=2 ...)
#set(USE_SDL_VERSION "3" CACHE STRING "SDL version to use (1, 2 or 3)")
# SDL Version Selection (set to 1, 2 or 3)
#set(USE_SDL_VERSION 1 CACHE STRING "SDL version to use (1, 2 or 3)")
set(USE_SDL_VERSION 1)
if(USE_SDL_VERSION EQUAL 3)
    # will find sdl3, sdl3-image, sdl3-mixer, sdl3-ttf, sdl3-gfx
elseif(USE_SDL_VERSION EQUAL 2)
    # will find sdl2, SDL2_mixer, SDL2_image, SDL2_ttf, SDL2_gfx
elseif(USE_SDL_VERSION EQUAL 1)
    # will find sdl, SDL_mixer, SDL_image, SDL_ttf, SDL_gfx
else()
    message(FATAL_ERROR "USE_SDL_VERSION must be 1, 2 or 3")
endif()
set_property(CACHE USE_SDL_VERSION PROPERTY STRINGS 1 2 3)

if(NOT USE_SDL_VERSION MATCHES "^[123]$")
    message(FATAL_ERROR "USE_SDL_VERSION must be 1, 2 or 3 (got '${USE_SDL_VERSION}')")
endif()

message(STATUS "Building rockbot with SDL${USE_SDL_VERSION}")

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

if(APPLE)
    # Match RockDroid.pro macosx deploy target (SDL macOS requires 10.7+)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "macOS deployment target")
endif()

# Find Dependencies
find_package(PkgConfig REQUIRED)

# Prefer the right pkg-config prefix per platform.
# Putting /usr/local first on macOS can pick an old FreeType that fails
# sdl3-ttf's Requires.private (freetype2 >= 20), hiding Homebrew packages.
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE BREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(BREW_PREFIX)
        set(ENV{PKG_CONFIG_PATH}
            "${BREW_PREFIX}/lib/pkgconfig:${BREW_PREFIX}/share/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
else()
    # Prefer /usr/local for self-built SDL3 over older system packages (Linux)
    set(ENV{PKG_CONFIG_PATH}
        "/usr/local/lib/pkgconfig:/usr/local/lib/x86_64-linux-gnu/pkgconfig:/usr/local/lib/aarch64-linux-gnu/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()

# Drop cached pkg-config results so switching USE_SDL_VERSION does not leak
# libraries from a previous configure (e.g. SDL2_gfx into an SDL3 build).
foreach(_pkg_prefix SDL SDL_MIXER SDL_IMAGE SDL_TTF SDL_GFX X11)
    foreach(_pkg_suffix FOUND INCLUDE_DIRS LIBRARIES LIBRARY_DIRS LDFLAGS CFLAGS CFLAGS_OTHER VERSION)
        unset(${_pkg_prefix}_${_pkg_suffix} CACHE)
        unset(${_pkg_prefix}_${_pkg_suffix})
    endforeach()
endforeach()

if(USE_SDL_VERSION EQUAL 3)
    pkg_check_modules(SDL REQUIRED sdl3)
    pkg_check_modules(SDL_MIXER REQUIRED sdl3-mixer)
    pkg_check_modules(SDL_IMAGE REQUIRED sdl3-image)
    pkg_check_modules(SDL_TTF REQUIRED sdl3-ttf)
    pkg_check_modules(SDL_GFX sdl3-gfx)
    if(SDL_GFX_FOUND)
        add_definitions(-DHAS_SDL3_GFX)
    else()
        message(STATUS "sdl3-gfx not found - zoom/rotate functions unavailable")
    endif()
    add_definitions(-DSDL3)
elseif(USE_SDL_VERSION EQUAL 2)
    pkg_check_modules(SDL REQUIRED sdl2)
    pkg_check_modules(SDL_MIXER REQUIRED SDL2_mixer)
    pkg_check_modules(SDL_IMAGE REQUIRED SDL2_image)
    pkg_check_modules(SDL_TTF REQUIRED SDL2_ttf)
    pkg_check_modules(SDL_GFX REQUIRED SDL2_gfx)
    add_definitions(-DSDL2)
else()
    pkg_check_modules(SDL REQUIRED sdl)
    pkg_check_modules(SDL_MIXER REQUIRED SDL_mixer)
    pkg_check_modules(SDL_IMAGE REQUIRED SDL_image)
    pkg_check_modules(SDL_TTF REQUIRED SDL_ttf)
    pkg_check_modules(SDL_GFX REQUIRED SDL_gfx)
endif()

# Platform defines / optional X11 (Linux only; RockDroid.pro links X11 on linux)
set(ROCKBOT_EXTRA_LIBS "")
if(APPLE)
    add_definitions(-DOSX -DPC)
elseif(UNIX)
    add_definitions(-DNES_RESOLUTION -DLINUX -DPC)
    pkg_check_modules(X11 REQUIRED x11)
    list(APPEND ROCKBOT_EXTRA_LIBS ${X11_LIBRARIES})
    list(APPEND ROCKBOT_EXTRA_LIBS dl)
else()
    add_definitions(-DPC)
endif()

# Include Directories
include_directories(
    .
    ${SDL_INCLUDE_DIRS}
    ${SDL_MIXER_INCLUDE_DIRS}
    ${SDL_IMAGE_INCLUDE_DIRS}
    ${SDL_TTF_INCLUDE_DIRS}
)
if(SDL_GFX_FOUND)
    include_directories(${SDL_GFX_INCLUDE_DIRS})
endif()
if(X11_FOUND)
    include_directories(${X11_INCLUDE_DIRS})
endif()

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
set(ROCKBOT_SDL_LIBS
    ${SDL_LDFLAGS}
    ${SDL_MIXER_LDFLAGS}
    ${SDL_IMAGE_LDFLAGS}
    ${SDL_TTF_LDFLAGS}
)
if(SDL_GFX_FOUND)
    list(APPEND ROCKBOT_SDL_LIBS ${SDL_GFX_LDFLAGS})
endif()

target_link_libraries(rockbot
    ${ROCKBOT_SDL_LIBS}
    ${ROCKBOT_EXTRA_LIBS}
)
