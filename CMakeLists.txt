cmake_minimum_required(VERSION 3.10)
project(rockbot)

# Compiler Settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output Directory (same location as DESTDIR=build used by the old qmake scripts)
if(CMAKE_SYSTEM_NAME STREQUAL "PS3" OR CMAKE_SYSTEM_NAME STREQUAL "NintendoWii" OR CMAKE_SYSTEM_NAME STREQUAL "NintendoGameCube" OR PSP OR VITA OR CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")
endif()

# PSP cross build (pspdev) uses psp/share/pspdev.cmake, which defines PSP.
if(PSP)
    set(ROCKBOT_PSP TRUE)
    # pspdev ships SDL2 (+ image/mixer/ttf/gfx); the game also builds with SDL2 there.
    set(USE_SDL_VERSION 2 CACHE STRING "SDL major version" FORCE)
endif()

# PS Vita cross build (VitaSDK) uses share/vita.toolchain.cmake, which defines VITA.
if(VITA)
    set(ROCKBOT_VITA TRUE)
    # vdpm ships SDL2 (+ image/mixer/ttf/gfx).
    set(USE_SDL_VERSION 2 CACHE STRING "SDL major version" FORCE)
endif()

# Nintendo Switch cross build (devkitA64 + libnx) uses devkitPro's Switch.cmake toolchain.
if(CMAKE_SYSTEM_NAME STREQUAL "NintendoSwitch")
    set(ROCKBOT_SWITCH TRUE)
    # switch-sdl2 and its image/mixer/ttf/gfx ports.
    set(USE_SDL_VERSION 2 CACHE STRING "SDL major version" FORCE)
endif()

# Buildroot handheld cross builds (SDL 1.2), selected with
#   -DROCKBOT_HANDHELD=dingux   (OpenDingux, mipsel; output rockbot_dingux.dge)
#   -DROCKBOT_HANDHELD=pocketgo (PocketGo/Bittboy, arm musl; output rockbot_pocketgo)
#   -DROCKBOT_HANDHELD=pandora  (OpenPandora, ARMv7 glibc 2.9, SDL 1.2; output rockbot_pandora)
# together with the SDK's share/buildroot/toolchainfile.cmake.
set(ROCKBOT_HANDHELD "" CACHE STRING "Handheld target: dingux, pocketgo or pandora")
set_property(CACHE ROCKBOT_HANDHELD PROPERTY STRINGS "" dingux pocketgo pandora)
if(ROCKBOT_HANDHELD AND NOT ROCKBOT_HANDHELD MATCHES "^(dingux|pocketgo|pandora)$")
    message(FATAL_ERROR "ROCKBOT_HANDHELD must be dingux, pocketgo or pandora")
endif()
if(ROCKBOT_HANDHELD)
    set(USE_SDL_VERSION 1 CACHE STRING "SDL major version" FORCE)
endif()

# PS2 cross build (ps2dev) uses ps2dev's share/ps2dev.cmake, which defines PS2.
if(PS2)
    set(ROCKBOT_PS2 TRUE)
    # The PS2 port runs on SDL 1.2 (libsdl / libsdlmixer in ps2sdk ports).
    set(USE_SDL_VERSION 1 CACHE STRING "SDL major version" FORCE)
endif()

# Nintendo Wii cross build (devkitPro devkitPPC + libogc) uses devkitPro's Wii.cmake toolchain.
# The GameCube build (GameCube.cmake) is the same port: same code paths (WII), plus GAMECUBE.
if(CMAKE_SYSTEM_NAME STREQUAL "NintendoWii" OR CMAKE_SYSTEM_NAME STREQUAL "NintendoGameCube")
    set(ROCKBOT_WII TRUE)
    if(CMAKE_SYSTEM_NAME STREQUAL "NintendoGameCube")
        set(ROCKBOT_GAMECUBE TRUE)
    endif()
    # The game's Wii code paths use SDL 1.2 (wii-sdl and friends).
    set(USE_SDL_VERSION 1 CACHE STRING "SDL major version" FORCE)
endif()

# PlayStation 3 cross build (PSL1GHT) uses its own toolchain file.
if(CMAKE_SYSTEM_NAME STREQUAL "PS3")
    set(ROCKBOT_PS3 TRUE)
    # PSL1GHT portlibs only ship SDL 1.2 (SDL_PSL1GHT).
    set(USE_SDL_VERSION 1 CACHE STRING "SDL major version" FORCE)
endif()

# Default to SDL1 if not specified
set(USE_SDL_VERSION 1 CACHE STRING "SDL major version")

# SDL Version Selection (override with: cmake -DUSE_SDL_VERSION=2 ...)
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
    )
    # MinGW often lacks libssp; keep stack protector for native Unix builds only.
    if(NOT WIN32 AND NOT ROCKBOT_PS3 AND NOT ROCKBOT_PSP AND NOT ROCKBOT_VITA AND NOT ROCKBOT_SWITCH AND NOT ROCKBOT_PS2 AND NOT ROCKBOT_HANDHELD AND NOT ROCKBOT_WII)
        add_compile_options(-fstack-protector-all -fstack-protector)
    endif()
endif()

if(ROCKBOT_PS2)
    # Same flags the qmake playstation2 config used (-G0 small data, no warnings
    # noise from the old code base).
    add_definitions(-DPLAYSTATION2=1 -D_EE)
    # ps2dev.cmake defines PS2, which the game never saw under qmake (it would
    # pull in the legacy fileio.h path); keep the old behavior.
    add_compile_options(-G0 -w -UPS2)
endif()

if(ROCKBOT_PSP)
    # Same flags the old Makefile.psp used: no exceptions/RTTI (code size and
    # RAM), small-data off for PRX builds.
    add_compile_options(-G0 -O2 -g)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions> $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
endif()

if(ROCKBOT_VITA)
    add_compile_options(-O2 -g)
endif()

if(ROCKBOT_SWITCH)
    add_compile_options(-O2 -g -ffunction-sections)
endif()

if(APPLE)
    # Match RockDroid.pro macosx deploy target (SDL macOS requires 10.7+)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "macOS deployment target")
endif()

if(ROCKBOT_PS3)
    # PSL1GHT portlibs. The prebuilt SDK's .pc files carry the CI build
    # prefix, so the paths are set explicitly instead of via pkg-config.
    set(SDL_INCLUDE_DIRS
        "${PORTLIBS}/include"
        "${PORTLIBS}/include/SDL"
        "${PSL1GHT}/ppu/include"
        "${PSL1GHT}/ppu/include/simdmath")
    set(SDL_MIXER_INCLUDE_DIRS "${PORTLIBS}/include/SDL")
    set(SDL_IMAGE_INCLUDE_DIRS "${PORTLIBS}/include/SDL")
    set(SDL_TTF_INCLUDE_DIRS "${PORTLIBS}/include/SDL")
    set(SDL_GFX_INCLUDE_DIRS "${PORTLIBS}/include/SDL")
    set(SDL_GFX_FOUND TRUE)
    add_definitions(-DPS3 -D__PSL1GHT__)
    set(ROCKBOT_EXTRA_LIBS
        ${PORTLIBS}/lib/libSDL_gfx.a
        ${PORTLIBS}/lib/libSDL_ttf.a
        ${PORTLIBS}/lib/libSDL_mixer.a
        ${PORTLIBS}/lib/libSDL_image.a
        ${PORTLIBS}/lib/libSDL.a
        ${PORTLIBS}/lib/libfreetype.a
        ${PORTLIBS}/lib/libpng.a
        ${PORTLIBS}/lib/libjpeg.a
        ${PORTLIBS}/lib/libtiff.a
        ${PORTLIBS}/lib/libvorbisfile.a
        ${PORTLIBS}/lib/libvorbis.a
        ${PORTLIBS}/lib/libogg.a
        ${PORTLIBS}/lib/libmikmod.a
        ${PORTLIBS}/lib/libmad.a
        ${PORTLIBS}/lib/libz.a
        -lm
        -lrt
        -llv2
        -lsysutil
        -lsysfs
        -lio
        -laudio
        -lgcm_sys
        -lrsx
    )
    set(ROCKBOT_SDL_LIBS "")
elseif(ROCKBOT_PS2)
    # ps2sdk ports are SDL 1.2 static libs without usable .pc files; ps2dev.cmake
    # already adds the ee/ports include and lib paths. SDL headers live in ports/include/SDL.
    set(PS2SDK "$ENV{PS2SDK}")
    set(SDL_INCLUDE_DIRS "${PS2SDK}/ports/include/SDL" "${PS2SDK}/ports/include")
    set(SDL_MIXER_INCLUDE_DIRS "")
    set(SDL_IMAGE_INCLUDE_DIRS "")
    set(SDL_TTF_INCLUDE_DIRS "")
    set(SDL_GFX_INCLUDE_DIRS "")
    set(SDL_GFX_FOUND TRUE)
    set(ROCKBOT_EXTRA_LIBS "")
else()
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
elseif(NOT CMAKE_CROSSCOMPILING)
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
elseif(DINGUX_A320_SDK OR PANDORA_SYSROOT)
    # Legacy SDKs (OpenDingux 1.2.5, OpenPandora): no usable pkg-config files, plain SDL 1.2 in the sysroot.
    if(PANDORA_SYSROOT)
        set(_legacy_root "${PANDORA_SYSROOT}")
    else()
        set(_legacy_root "${DINGUX_A320_SDK}")
    endif()
    set(SDL_INCLUDE_DIRS "${_legacy_root}/usr/include/SDL" "${_legacy_root}/usr/include")
    set(SDL_LDFLAGS -lSDL)
    set(SDL_MIXER_LDFLAGS -lSDL_mixer)
    set(SDL_IMAGE_LDFLAGS -lSDL_image)
    set(SDL_TTF_LDFLAGS -lSDL_ttf)
    set(SDL_GFX_LDFLAGS -lSDL_gfx)
    add_definitions(-D_GNU_SOURCE=1 -DDINGUX_LEGACY_SDL_TTF)
else()
    pkg_check_modules(SDL REQUIRED sdl)
    pkg_check_modules(SDL_MIXER REQUIRED SDL_mixer)
    pkg_check_modules(SDL_IMAGE REQUIRED SDL_image)
    pkg_check_modules(SDL_TTF REQUIRED SDL_ttf)
    pkg_check_modules(SDL_GFX REQUIRED SDL_gfx)
endif()

# Platform defines / optional X11 (Linux only; RockDroid.pro links X11 on linux)
set(ROCKBOT_EXTRA_LIBS "")
if(WIN32)
    # Codebase uses WIN32 (not only the compiler's _WIN32).
    add_definitions(-DWIN32 -DPC)
elseif(APPLE)
    add_definitions(-DOSX -DPC)
elseif(ROCKBOT_PSP)
    # -DPSP / -D__PSP__ come from pspdev.cmake. SDL_MAIN_HANDLED keeps the
    # real main() for the PSP crt0 (SDL2 would rename it to SDL_main).
    add_definitions(-DHANDHELD -DSDL_MAIN_HANDLED)
    set(ROCKBOT_PSP_POISON OFF CACHE BOOL "Fill C++ allocations with 0xCD (debug aid for PSP)")
    if(ROCKBOT_PSP_POISON)
        add_definitions(-DROCKBOT_PSP_POISON)
    endif()
elseif(ROCKBOT_WII)
    add_definitions(-DWII -DHANDHELD)
    if(ROCKBOT_GAMECUBE)
        add_definitions(-DGAMECUBE)
    endif()
elseif(ROCKBOT_SWITCH)
    add_definitions(-DSWITCH -DHANDHELD -D__SWITCH__)
elseif(ROCKBOT_VITA)
    # VITA also selects the PS Vita code paths; SDL_MAIN_HANDLED is not used (SDL2 provides SDL_main).
    add_definitions(-DVITA -DHANDHELD)
elseif(ROCKBOT_HANDHELD STREQUAL "dingux")
    add_definitions(-DDINGUX -DHANDHELD -D_REENTRANT)
elseif(ROCKBOT_HANDHELD STREQUAL "pandora")
    add_definitions(-DOPEN_PANDORA -DHANDHELD -D_REENTRANT)
elseif(ROCKBOT_HANDHELD STREQUAL "pocketgo")
    add_definitions(-DPOCKETGO -DHANDHELD -DHAVE_STDINT_H -DVERSION_BITTBOY -D_REENTRANT)
elseif(UNIX)
    add_definitions(-DNES_RESOLUTION -DLINUX -DPC)
    pkg_check_modules(X11 REQUIRED x11)
    list(APPEND ROCKBOT_EXTRA_LIBS ${X11_LIBRARIES})
    list(APPEND ROCKBOT_EXTRA_LIBS dl)
else()
    add_definitions(-DPC)
endif()
endif() # ROCKBOT_PS3

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
    ports/ps3/ps3_platform.cpp
    sdl_layer.cpp
)
if(ROCKBOT_PSP)
    list(APPEND SOURCES ports/psp/psp_platform.cpp ports/psp/psp_music.cpp)
endif()

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
    ports/ps3/ps3_platform.h
    ports/psp/psp_platform.h
    ports/psp/psp_music.h
    sdl_layer.h
)

add_executable(rockbot ${SOURCES} ${HEADERS})

# Every build writes rockbot_<platform><ext> (rockbot_playstation2.elf, rockbot_dingux.dge, ...), so binaries
# of different platforms never share a name. The package scripts rename it where a loader demands a fixed
# name (EBOOT.PBP, EBOOT.BIN, eboot.bin, boot.dol).
if(ROCKBOT_PS2)
    set(ROCKBOT_PLATFORM_NAME playstation2)
elseif(ROCKBOT_PSP)
    set(ROCKBOT_PLATFORM_NAME psp)
elseif(ROCKBOT_PS3)
    set(ROCKBOT_PLATFORM_NAME playstation3)
elseif(ROCKBOT_VITA)
    set(ROCKBOT_PLATFORM_NAME vita)
elseif(ROCKBOT_SWITCH)
    set(ROCKBOT_PLATFORM_NAME switch)
elseif(ROCKBOT_GAMECUBE)
    set(ROCKBOT_PLATFORM_NAME gamecube)
elseif(ROCKBOT_WII)
    set(ROCKBOT_PLATFORM_NAME wii)
elseif(ROCKBOT_HANDHELD)
    set(ROCKBOT_PLATFORM_NAME ${ROCKBOT_HANDHELD})
elseif(WIN32)
    set(ROCKBOT_PLATFORM_NAME windows)
elseif(APPLE)
    set(ROCKBOT_PLATFORM_NAME osx)
else()
    set(ROCKBOT_PLATFORM_NAME linux)
endif()
set_target_properties(rockbot PROPERTIES OUTPUT_NAME rockbot_${ROCKBOT_PLATFORM_NAME})

# Link Libraries
if(ROCKBOT_WII)
    # Static devkitPro portlibs: take the transitive dependencies from pkg-config, then the
    # libogc pieces the game uses directly (SD card, Wii remote, keyboard).
    set(ROCKBOT_SDL_LIBS
        ${SDL_GFX_STATIC_LDFLAGS} ${SDL_TTF_STATIC_LDFLAGS} ${SDL_MIXER_STATIC_LDFLAGS}
        ${SDL_IMAGE_STATIC_LDFLAGS} ${SDL_STATIC_LDFLAGS}
        -lbz2 -lbrotlidec -lbrotlicommon -lz)
    if(ROCKBOT_GAMECUBE)
        # No Wii remote / Bluetooth / USB keyboard on the GameCube; the SD card is an SD Gecko / SD2SP2.
        list(APPEND ROCKBOT_SDL_LIBS -lfat -logc -lm)
    else()
        list(APPEND ROCKBOT_SDL_LIBS -lfat -lwiiuse -lbte -lwiikeyboard -logc -lm)
    endif()
elseif(ROCKBOT_HANDHELD STREQUAL "dingux")
    set(ROCKBOT_SDL_LIBS ${SDL_LDFLAGS} ${SDL_MIXER_LDFLAGS} ${SDL_IMAGE_LDFLAGS}
        ${SDL_TTF_LDFLAGS} ${SDL_GFX_LDFLAGS} -lpthread)
    # The toolchain's libstdc++ is newer than the one on the device (OpenDingux images differ):
    # link it statically or the loader fails with "can't resolve symbol ..._M_replace...".
    target_link_options(rockbot PRIVATE -static-libstdc++ -static-libgcc)
    set_target_properties(rockbot PROPERTIES SUFFIX ".dge")
elseif(ROCKBOT_HANDHELD STREQUAL "pandora")
    set(ROCKBOT_SDL_LIBS ${SDL_LDFLAGS} ${SDL_MIXER_LDFLAGS} ${SDL_IMAGE_LDFLAGS}
        ${SDL_TTF_LDFLAGS} ${SDL_GFX_LDFLAGS} -lpthread)
    # The Sourcery 4.6 libstdc++ is newer than the one on the device: link it statically.
    target_link_options(rockbot PRIVATE -static-libstdc++ -static-libgcc)
    set_target_properties(rockbot PROPERTIES SUFFIX "")
elseif(ROCKBOT_HANDHELD STREQUAL "pocketgo")
    # The SDK only has static libs; the .pc files miss some transitive deps,
    # so add the ones the old qmake config listed and let the linker resolve cycles.
    set(ROCKBOT_SDL_LIBS
        -Wl,--start-group
        ${SDL_MIXER_STATIC_LDFLAGS} ${SDL_IMAGE_STATIC_LDFLAGS} ${SDL_TTF_STATIC_LDFLAGS}
        ${SDL_GFX_STATIC_LDFLAGS} ${SDL_STATIC_LDFLAGS}
        -lpng -ljpeg -lfreetype -lvorbisfile -lvorbis -logg -lmikmod -lmad -lasound
        -lm -pthread -lz -lstdc++
        -Wl,--end-group)
    target_link_options(rockbot PRIVATE -static-libstdc++ -static-libgcc)
    set_target_properties(rockbot PROPERTIES SUFFIX "")
elseif(ROCKBOT_PS2)
    # Static libs have circular references (SDL <-> gsKit <-> ps2sdk); group them.
    # Order follows the old qmake config.
    set(ROCKBOT_SDL_LIBS
        -Wl,--start-group
        -lSDL_gfx -lSDL_image -ljpeg -ltiff -lpng -lz -ldebug -lSDL_ttf
        -lsdlmixer -lsdlmain -lsdl -lgskit -ldmakit -lpad -llzma -lfreetype
        -lm -lcdvd -lpacket -ldma -lmc -lstdc++ -lc -lkernel
        -Wl,--end-group
    )
    # rockbot_playstation2.elf, next to the other build output (build/packages/PS2.sh reads it).
    set_target_properties(rockbot PROPERTIES SUFFIX ".elf")
elseif(ROCKBOT_PSP)
    # Static link: the PSP has no shared libs, so take the transitive deps too.
    set(ROCKBOT_SDL_LIBS
        ${SDL_MIXER_STATIC_LDFLAGS}
        ${SDL_IMAGE_STATIC_LDFLAGS}
        ${SDL_TTF_STATIC_LDFLAGS}
        ${SDL_GFX_STATIC_LDFLAGS}
        ${SDL_STATIC_LDFLAGS}
    )
    # harfbuzz.pc asks for -pthread, which psp-gcc does not know (PSP has its own threads).
    list(REMOVE_ITEM ROCKBOT_SDL_LIBS -pthread)
    # libxmp-lite plays the tracker music (libmodplug crashes the real PSP, see ports/psp/psp_music.h)
    list(APPEND ROCKBOT_SDL_LIBS -lxmp-lite)
    # PSP SDK (stub) libraries must come last or psp-fixup-imports warns about
    # out-of-order stubs. Pull them out of the pkg-config output and re-append
    # them after the C/C++ runtime.
    set(_psp_sdk_libs -lGL -lpspvram -lpspaudio -lpspvfpu -lpspdisplay -lpspgu
        -lpspge -lpsphprm -lpspctrl -lpsppower -lpspdebug -lpsprtc)
    list(REMOVE_ITEM ROCKBOT_SDL_LIBS ${_psp_sdk_libs})
    # CMAKE_CXX_STANDARD_LIBRARIES is emitted at the very end of the link line.
    string(REPLACE ";" " " _psp_sdk_libs_str "${_psp_sdk_libs}")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-lstdc++ -lm -lc ${_psp_sdk_libs_str}")
elseif(ROCKBOT_SWITCH)
    # Static portlibs: transitive dependencies come from the .pc files; libnx goes last.
    set(ROCKBOT_SDL_LIBS
        ${SDL_MIXER_STATIC_LDFLAGS}
        ${SDL_IMAGE_STATIC_LDFLAGS}
        ${SDL_TTF_STATIC_LDFLAGS}
        ${SDL_GFX_STATIC_LDFLAGS}
        ${SDL_STATIC_LDFLAGS}
    )
    list(REMOVE_ITEM ROCKBOT_SDL_LIBS -pthread)
elseif(ROCKBOT_VITA)
    # Static link; sdl2.pc (Libs.private) pulls the Vita system stubs in the right order.
    set(ROCKBOT_SDL_LIBS
        ${SDL_MIXER_STATIC_LDFLAGS}
        ${SDL_IMAGE_STATIC_LDFLAGS}
        ${SDL_TTF_STATIC_LDFLAGS}
        ${SDL_GFX_STATIC_LDFLAGS}
        ${SDL_STATIC_LDFLAGS}
    )
    list(REMOVE_ITEM ROCKBOT_SDL_LIBS -pthread)
elseif(NOT ROCKBOT_PS3)
    set(ROCKBOT_SDL_LIBS
        ${SDL_LDFLAGS}
        ${SDL_MIXER_LDFLAGS}
        ${SDL_IMAGE_LDFLAGS}
        ${SDL_TTF_LDFLAGS}
    )
    if(SDL_GFX_FOUND)
        list(APPEND ROCKBOT_SDL_LIBS ${SDL_GFX_LDFLAGS})
    endif()
endif()

if(ROCKBOT_PS3)
    # Static PSL1GHT libs have circular references; group them.
    target_link_libraries(rockbot
        -Wl,--start-group
        ${ROCKBOT_SDL_LIBS}
        ${ROCKBOT_EXTRA_LIBS}
        -Wl,--end-group
    )
    # Do not use --gc-sections here: it can drop PSL1GHT's lv2-crt0.o _start
    # entry point, leaving the ELF with no valid entry address.
    set_target_properties(rockbot PROPERTIES
        LINK_FLAGS "-Wl,-Map=${CMAKE_BINARY_DIR}/rockbot.map")
else()
    target_link_libraries(rockbot
        ${ROCKBOT_SDL_LIBS}
        ${ROCKBOT_EXTRA_LIBS}
    )
endif()

if(ROCKBOT_PSP)
    # EBOOT.PBP next to the binary: PRX build, all RAM (MEMSIZE=1), same as the
    # old Makefile.psp (BUILD_PRX + PSP_LARGE_MEMORY).
    set(ROCKBOT_VERSION_NAME "0.0.0" CACHE STRING "Version shown in the PSP menu")
    set(ROCKBOT_VERSION_NUMBER 1 CACHE STRING "Game number (1 = RockDroid1, 2 = RockDroid2)")
    set(_psp_files "${CMAKE_CURRENT_SOURCE_DIR}/build/packages/files")
    if(ROCKBOT_VERSION_NUMBER EQUAL 1)
        set(_psp_icon "${_psp_files}/psp_icon1.png")
        set(_psp_bg "${_psp_files}/psp_background1.png")
    else()
        set(_psp_icon "${_psp_files}/psp_icon.png")
        set(_psp_bg "${_psp_files}/psp_background.png")
    endif()
    create_pbp_file(
        TARGET rockbot
        BUILD_PRX
        TITLE "Rockbot ${ROCKBOT_VERSION_NAME}"
        VERSION "01.00"
        MEMSIZE 1
        ICON_PATH "${_psp_icon}"
        BACKGROUND_PATH "${_psp_bg}"
    )
endif()

if(ROCKBOT_SWITCH)
    # rockbot_switch.nro next to the ELF; ports/switch/build.sh adds the game data folders beside it.
    set(ROCKBOT_NRO_ICON "" CACHE FILEPATH "256x256 JPEG icon for the Homebrew Menu (made by ports/switch/build.sh)")
    set(_nx_icon "${ROCKBOT_NRO_ICON}")
    set(ROCKBOT_VERSION_NAME "0.0.0" CACHE STRING "Version shown in the Homebrew Menu")
    set(ROCKBOT_VERSION_NUMBER 1 CACHE STRING "Game number (1 = RockDroid1, 2 = RockDroid2)")
    nx_generate_nacp(OUTPUT rockbot.nacp NAME "Rockbot ${ROCKBOT_VERSION_NUMBER}" AUTHOR "Upperland" VERSION "${ROCKBOT_VERSION_NAME}")
    if(_nx_icon AND EXISTS "${_nx_icon}")
        nx_create_nro(rockbot OUTPUT rockbot_switch.nro NACP rockbot.nacp ICON "${_nx_icon}")
    else()
        nx_create_nro(rockbot OUTPUT rockbot_switch.nro NACP rockbot.nacp)
    endif()
endif()

if(ROCKBOT_VITA)
    # eboot.bin (a compressed SELF) next to the ELF; ports/vita/build.sh packs it into the VPK.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-q")
    include("${VITASDK}/share/vita.cmake" REQUIRED)
    vita_create_self(eboot.bin rockbot UNSAFE)
endif()

if(ROCKBOT_WII)
    # rockbot_wii.dol / rockbot_gamecube.dol next to the ELF; the Wii package names it boot.dol, the
    # file the Homebrew Channel loads.
    add_custom_command(TARGET rockbot POST_BUILD
        COMMAND ${ELF2DOL_EXE} "$<TARGET_FILE:rockbot>" "${CMAKE_BINARY_DIR}/rockbot_${ROCKBOT_PLATFORM_NAME}.dol"
        COMMENT "Creating rockbot_${ROCKBOT_PLATFORM_NAME}.dol")
endif()
