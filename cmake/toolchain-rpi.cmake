# Raspberry Pi OS cross toolchain (Debian multiarch), used inside ports/raspberry/Dockerfile.
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-rpi.cmake -DRPI_ARCH=armhf|arm64 ...

# The toolchain file is evaluated more than once during configure and -DRPI_ARCH is not always
# visible in the later evaluations, so the RPI_ARCH environment variable (set by
# build-raspberry.sh) is honoured too; otherwise the pkg-config wrapper would flip back to armhf.
if(NOT RPI_ARCH AND DEFINED ENV{RPI_ARCH})
    set(RPI_ARCH "$ENV{RPI_ARCH}")
endif()
if(NOT RPI_ARCH)
    set(RPI_ARCH "armhf")
endif()
if(RPI_ARCH STREQUAL "arm64")
    set(RPI_TRIPLET "aarch64-linux-gnu")
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
else()
    set(RPI_TRIPLET "arm-linux-gnueabihf")
    set(CMAKE_SYSTEM_PROCESSOR arm)
endif()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER   "${RPI_TRIPLET}-gcc")
set(CMAKE_CXX_COMPILER "${RPI_TRIPLET}-g++")
set(CMAKE_STRIP        "${RPI_TRIPLET}-strip")

set(CMAKE_FIND_ROOT_PATH "/usr/${RPI_TRIPLET}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# pkg-config must look at the target architecture's .pc files, not the host's. A wrapper sets
# PKG_CONFIG_LIBDIR itself on every call (plain set(ENV{...}) was overridden during configure).
set(ENV{RPI_TRIPLET} "${RPI_TRIPLET}")
set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_LIST_DIR}/rpi-pkg-config.sh" CACHE FILEPATH "pkg-config wrapper" FORCE)
