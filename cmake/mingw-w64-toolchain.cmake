# MinGW-w64 cross-compile toolchain (used by build-windows.sh from Linux).
#
# Required cache vars (pass with -D):
#   MINGW_TARGET_TRIPLET  e.g. x86_64-w64-mingw32 or i686-w64-mingw32
#   MINGW_SDL_PREFIX      absolute path to installed SDL mingw root

if(NOT MINGW_TARGET_TRIPLET)
    set(MINGW_TARGET_TRIPLET "x86_64-w64-mingw32" CACHE STRING "MinGW target triplet")
endif()

if(NOT MINGW_SDL_PREFIX)
    message(FATAL_ERROR "MINGW_SDL_PREFIX must point to the MinGW SDL install prefix")
endif()

set(CMAKE_SYSTEM_NAME Windows)
if(MINGW_TARGET_TRIPLET MATCHES "^i686")
    set(CMAKE_SYSTEM_PROCESSOR x86)
else()
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif()

set(CMAKE_C_COMPILER   "${MINGW_TARGET_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${MINGW_TARGET_TRIPLET}-g++")
set(CMAKE_RC_COMPILER  "${MINGW_TARGET_TRIPLET}-windres")
set(CMAKE_ADDR2LINE    "${MINGW_TARGET_TRIPLET}-addr2line")
set(CMAKE_AR           "${MINGW_TARGET_TRIPLET}-ar")
set(CMAKE_NM           "${MINGW_TARGET_TRIPLET}-nm")
set(CMAKE_OBJCOPY      "${MINGW_TARGET_TRIPLET}-objcopy")
set(CMAKE_OBJDUMP      "${MINGW_TARGET_TRIPLET}-objdump")
set(CMAKE_RANLIB       "${MINGW_TARGET_TRIPLET}-ranlib")
set(CMAKE_STRIP        "${MINGW_TARGET_TRIPLET}-strip")

set(CMAKE_FIND_ROOT_PATH
    "/usr/${MINGW_TARGET_TRIPLET}"
    "${MINGW_SDL_PREFIX}"
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_PREFIX_PATH "${MINGW_SDL_PREFIX}" CACHE PATH "" FORCE)
set(ENV{PKG_CONFIG_LIBDIR} "${MINGW_SDL_PREFIX}/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")
set(PKG_CONFIG_EXECUTABLE "pkg-config" CACHE FILEPATH "" FORCE)

# Prefer a mostly-static C++ runtime so fewer MinGW DLLs must be shipped.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++")
