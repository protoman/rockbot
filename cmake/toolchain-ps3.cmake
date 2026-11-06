# PlayStation 3 cross toolchain (PSL1GHT / ps3dev).
#
# Usage:
#   cmake -B res/ps3 -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-ps3.cmake
#
# The ps3dev environment is located through PS3DEV (default:
# /usr/local/ps3dev). Point it somewhere else with -DPS3DEV=/path or the
# PS3DEV environment variable.

set(CMAKE_SYSTEM_NAME PS3)
set(CMAKE_SYSTEM_PROCESSOR ppu)
set(CMAKE_SYSTEM_VERSION 1)

if(NOT PS3DEV)
    if(DEFINED ENV{PS3DEV})
        set(PS3DEV "$ENV{PS3DEV}")
    else()
        set(PS3DEV "/usr/local/ps3dev")
    endif()
endif()

set(PS3DEV "${PS3DEV}" CACHE PATH "ps3dev installation prefix")
set(PSL1GHT "${PS3DEV}" CACHE PATH "PSL1GHT installation prefix")
set(PORTLIBS "${PS3DEV}/portlibs/ppu" CACHE PATH "ps3dev portlibs prefix")

set(CMAKE_C_COMPILER "${PS3DEV}/ppu/bin/ppu-gcc")
set(CMAKE_CXX_COMPILER "${PS3DEV}/ppu/bin/ppu-g++")
set(CMAKE_AR "${PS3DEV}/ppu/bin/ppu-ar" CACHE FILEPATH "")
set(CMAKE_RANLIB "${PS3DEV}/ppu/bin/ppu-ranlib" CACHE FILEPATH "")
set(CMAKE_STRIP "${PS3DEV}/ppu/bin/ppu-strip" CACHE FILEPATH "")

# Older PSL1GHT GCC builds need the SDK path both in the environment and in
# the search paths (the compiler driver reads PSL1GHT when it starts).
set(ENV{PS3DEV} "${PS3DEV}")
set(ENV{PSL1GHT} "${PS3DEV}")

set(CMAKE_C_FLAGS_INIT "-mcpu=cell -mhard-float -ffunction-sections -fdata-sections")
# PSL1GHT's static libstdc++ exception runtime hangs during __libc_init when
# the binary has a large .eh_frame. Rockbot does not use C++ exceptions or
# RTTI, so disable both and keep the startup path small.
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cell -mhard-float -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-omit-frame-pointer -D_GLIBCXX11_USE_C99_STDIO")

set(CMAKE_FIND_ROOT_PATH "${PS3DEV}" "${PORTLIBS}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
