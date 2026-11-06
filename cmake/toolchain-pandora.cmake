# OpenPandora (OMAP3530, Cortex-A8, glibc 2.9, SDL 1.2).
# Compiler: GCC built by ports/pandora/build-toolchain.sh (PANDORA_TOOLCHAIN = its install prefix, the
# folder with bin/arm-angstrom-linux-gnueabi-gcc). Headers and libraries: the OpenPandora SDK sysroot
# (PANDORA_SYSROOT), which the compiler was configured with.
foreach(v PANDORA_TOOLCHAIN PANDORA_SYSROOT)
    if(NOT ${v})
        set(${v} "$ENV{${v}}")
    endif()
endforeach()
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER   "${PANDORA_TOOLCHAIN}/bin/arm-angstrom-linux-gnueabi-gcc")
set(CMAKE_CXX_COMPILER "${PANDORA_TOOLCHAIN}/bin/arm-angstrom-linux-gnueabi-g++")
set(CMAKE_SYSROOT "${PANDORA_SYSROOT}")
set(_pandora_flags "-march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp -fsigned-char")
set(CMAKE_C_FLAGS_INIT "${_pandora_flags}")
set(CMAKE_CXX_FLAGS_INIT "${_pandora_flags}")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-rpath-link,${PANDORA_SYSROOT}/usr/lib -Wl,-rpath-link,${PANDORA_SYSROOT}/lib")
set(CMAKE_FIND_ROOT_PATH "${PANDORA_SYSROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
