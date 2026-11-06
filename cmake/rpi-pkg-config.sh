#!/bin/sh
# pkg-config for the Raspberry Pi cross build: always look at the target architecture's .pc files.
# RPI_TRIPLET is set by build-raspberry.sh (aarch64-linux-gnu or arm-linux-gnueabihf).
exec env PKG_CONFIG_LIBDIR="/usr/lib/${RPI_TRIPLET:-arm-linux-gnueabihf}/pkgconfig:/usr/share/pkgconfig" PKG_CONFIG_PATH= \
    pkg-config "$@"
