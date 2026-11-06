# Dingoo A320 (JZ4740, MIPS32r1, soft-float) with the legacy OpenDingux 1.2.5 toolchain
# (GCC 4.7.1, uClibc 0.9.31). Set DINGUX_A320_SDK to the toolchain root (the folder that holds usr/).
if(NOT DINGUX_A320_SDK)
    set(DINGUX_A320_SDK "$ENV{DINGUX_A320_SDK}")
endif()
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(CMAKE_C_COMPILER   "${DINGUX_A320_SDK}/usr/bin/mipsel-linux-gcc")
set(CMAKE_CXX_COMPILER "${DINGUX_A320_SDK}/usr/bin/mipsel-linux-g++")
set(CMAKE_SYSROOT "${DINGUX_A320_SDK}")
set(CMAKE_C_FLAGS_INIT "--sysroot=${DINGUX_A320_SDK}")
set(CMAKE_CXX_FLAGS_INIT "--sysroot=${DINGUX_A320_SDK}")
set(CMAKE_FIND_ROOT_PATH "${DINGUX_A320_SDK}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# JZ4740 is MIPS32r1 without an FPU: no r2 instructions, soft-float.
set(CMAKE_C_FLAGS_INIT "--sysroot=${DINGUX_A320_SDK} -march=mips32 -mtune=mips32 -msoft-float")
set(CMAKE_CXX_FLAGS_INIT "--sysroot=${DINGUX_A320_SDK} -march=mips32 -mtune=mips32 -msoft-float")
