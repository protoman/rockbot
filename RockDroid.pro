#-------------------------------------------------
#
# Project created by QtCreator 2011-03-05T22:53:17
#
#-------------------------------------------------

CONFIG -= qt
CONFIG -= linux # qtcreator adds linux even if shouldn't, so we remove
QT       -= core
QT       -= gui

CONFIG += debug

CONFIG += linux

#CONFIG += macosx

#CONFIG += win32

#CONFIG += raspberry

#CONFIG += dingux

#CONFIG += pocketgo

#CONFIG += playstation2

#CONFIG += playstation3

#CONFIG += psp
    # try to replicate the exact same behavior of the makefile.psp

#CONFIG += open_pandora

#CONFIG += wii
    # missing rotozoom, check if we can remove it (I think it is used for missiles)

#CONFIG += dreamcast
    # /opt/toolchains/dc/sh-elf/bin/sh-elf-g++: Command not found


#DEFINES+= ANDROID=1

# -m64 problem in android build:
# NOTE for android build on 64: /usr/share/qt4/mkspecs/default/qmake.conf
# or, in QT5, /usr/lib/x86_64-linux-gnu/qt5/mkspecs/linux-g++-64/qmake.conf
# or, in a install in home (qt5),


### BETA VERSION ###
#DEFINESLIST += -DBETA_VERSION
#DEFINES += BETA_VERSION=1


CONFIG += console
CONFIG -= app_bundle
TARGET = rockbot

linux {
    DEFINES += LINUX
    LIBS += -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `sdl-config --libs` -ldl -lstdc++ -fstack-protector-all
    INCLUDES += -I/usr/include/SDL -I/usr/include -I. -I./include -L/usr/lib

    # SDL2
    #DEFINES += LINUX SDL2
    #LIBS += -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx `sdl2-config --libs` -ldl -lstdc++ -fstack-protector-all
    #INCLUDES += -I/usr/include -I. -I./include -L/usr/lib

    QMAKE_CCFLAGS += -std=c++17 -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive -Werror=return-type -fstack-protector-all -fstack-protector
    QMAKE_CXXFLAGS += -std=c++17 -DLINUX -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive -Werror=return-type -fstack-protector-all -fstack-protector
}

macosx {
    DEFINES += OSX
    LIBS += `sdl2-config --libs`
    LIBS += -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2_gfx
#    DESTDIR = ../

#    LIBS += -framework OpenGL

    INCLUDES = -I/opt/homebrew/include -I/opt/homebrew/opt/qt@5 `sdl2-config --cflags` -I.

    INCLUDEPATH += /opt/homebrew/include /opt/homebrew/opt/qt@5
    
    QMAKE_CCFLAGS += -DOSX -DPC
    QMAKE_CXXFLAGS += -DOSX -DPC
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    # xcodebuild -showsdks
    # error: SDL for Mac OS X only supports deploying on 10.7 and above.
    QMAKE_MAC_SDK = macosx15.5
    CONFIG -= console
}

win32 {
    DEFINES += WIN32
    LIBS =  -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx -lmingw32 -lSDL -mwindows -LC:\Qt\5.15.2\mingw81_32\lib -LC:\Qt\5.15.2\mingw81_32\lib\x86 -L/usr/lib -L/usr/lib/x86 -LC:\MinGW\lib -LC:\MinGW\lib\x86
    INCLUDES = -I/usr/include/SDL -I/usr/include -I. -I./include -L/usr/lib
    QMAKE_CCFLAGS += -O2 -DWIN32 -DPC
    QMAKE_CXXFLAGS += -O2 -DWIN32 -DPC -IC:\Qt\5.15.2\mingw81_32\ -IC:\Qt\5.15.2\mingw81_32\include -LC:\Qt\5.15.2\mingw81_32\lib -LC:\Qt\5.15.2\mingw81_32\lib\x86
    CONFIG -= console
}

raspberry {
    DEFINES+= RASPBERRY=1
    TARGET = rockbot_pi
    LIBS = -L/usr/X11R6/lib -lX11 -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `sdl-config --libs` -ldl
    INCLUDES = -I/usr/include/SDL -I/usr/include -I. -I./include -L/usr/lib
    QMAKE_CCFLAGS += -O2 -DRASPBERRY -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive
    QMAKE_CXXFLAGS += -O2 -DRASPBERRY -DPC -Wno-reorder -Wno-ignored-qualifiers -fpermissive
}

dingux {
    DEFINES += DINGUX
    TARGET = rockbot.dge

    QMAKE_CC = /media/iuri/SamsungEXT4/development/SDK/opendingux/usr/bin/mipsel-linux-gcc
    QMAKE_CXX = /media/iuri/SamsungEXT4/development/SDK/opendingux/usr/bin/mipsel-linux-g++
    QMAKE_LINK = /media/iuri/SamsungEXT4/development/SDK/opendingux/usr/bin/mipsel-linux-g++

    QMAKE_CFLAGS += -pipe -g -Wall -W -D_REENTRANT -DDINGUX -DHANDHELD -O3
    QMAKE_CXXFLAGS += -I/media/iuri/SamsungEXT4/development/SDK/opendingux/usr/include -pipe -g -Wall -W -D_REENTRANT -DDINGUX -DHANDHELD -O3 -L/media/iuri/SamsungEXT4/development/SDK/opendingux/usr/lib

    LIBS = $(SUBLIBS) -L/media/iuri/SamsungEXT4/development/SDK/opendingux/usr/lib/ -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `/media/iuri/SamsungEXT4/development/SDK/opendingux/usr/bin/sdl-config --libs` -lpthread
    #-o /media/iuri/SamsungEXT4/development/SDK/opendingux/usr/lib/crt1.o

    INCLUDES = -I/media/iuri/SamsungEXT4/development/SDK/opendingux/usr/include -I. -I../include -I.

    #QMAKE_POST_LINK += /media/iuri/SamsungEXT4/development/SDK/opendingux/usr/bin/mipsel-linux-strip --strip-all rockbot.dge
}

pocketgo {
    DEFINES += POCKETGO
    DEFINES += HAVE_STDINT_H
    DEFINES += VERSION_BITTBOY
    TARGET = rockbot_pocketgo
    POCKETGO_SDK = /media/iuri/SamsungEXT4/development/SDK/PocketGo/output/host

    QMAKE_CC = $${POCKETGO_SDK}/bin/arm-buildroot-linux-musleabi-gcc
    QMAKE_CXX = $${POCKETGO_SDK}/bin/arm-buildroot-linux-musleabi-g++
    QMAKE_LINK = $${POCKETGO_SDK}/bin/arm-buildroot-linux-musleabi-g++

    QMAKE_CFLAGS += $(INCLUDES) $(OPT_FLAGS) -std=cnu11 -pipe -g -Wall -W -D_REENTRANT -DPOCKETGO -DHANDHELD -O3
    QMAKE_CXXFLAGS += $(INCLUDES) $(OPT_FLAGS) -std=c++11 -pipe -g -Wall -W -D_REENTRANT -DPOCKETGO -DHANDHELD -O3
    QMAKE_CXXFLAGS -= -ccc-gcc-name g++

    LIBS = -L$${POCKETGO_SDK}/arm-buildroot-linux-musleabi/sysroot/usr/lib -Wl,--start-group -lSDL -lSDL_image -lpng -ljpeg -lSDL_mixer -lasound -lmikmod -logg -lvorbis -lvorbisfile -lmad -lfreetype -lSDL_ttf -lSDL_gfx -lm -pthread -lz -lstdc++ $(EXTRA_LDFLAGS) -Wl,--end-group

    INCLUDES = -I$${POCKETGO_SDK}/arm-buildroot-linux-musleabi/sysroot/usr/bin/../../usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I$${POCKETGO_SDK}/arm-buildroot-linux-musleabi/sysroot/include -I$${POCKETGO_SDK}/include -I. -I../include -I. $(SDL_CFLAGS) -I/media/iuri/SamsungEXT4/development/SDK/PocketGo/output/host/arm-buildroot-linux-musleabi/sysroot/usr/include

}

playstation2 {
    DEFINES += PLAYSTATION2=1
    DEFINES += _EE
    #DEFINES += _IOP
    TARGET = rockbot.elf

    PS2DEV = /media/iuri/SamsungEXT4/development/SDK/PS2/sdk
    PS2SDK = $${PS2DEV}/ps2sdk
    GSKIT = $${PS2DEV}/gsKit

    export(PS2DEV)
    export(PS2SDK)
    export(GSKIT)

    # pre-build commands
    SJPCM.target = SJPCM.s
    SJPCM.commands = $${PS2SDK}/bin/bin2s ../ports/ps2/SJPCM.irx ../ports/ps2/SJPCM.s SJPCM
    cdvd.target = cdvd.s
    cdvd.commands = $${PS2SDK}/bin/bin2s ../ports/ps2/cdvd.irx ../ports/ps2/cdvd.s cdvd
    usbd.target = usbd.s
    usbd.commands = $${PS2SDK}/bin/bin2s ../ports/ps2/usbd.irx ../ports/ps2/usbd.s usbd
    usbhdfsd.target = usbhdfsd.s
    usbhdfsd.commands = $${PS2SDK}/bin/bin2s ../ports/ps2/usbhdfsd.irx ../ports/ps2/usbhdfsd.s usbhdfsd
    QMAKE_EXTRA_TARGETS += SJPCM cdvd usbd usbhdfsd
    PRE_TARGETDEPS += SJPCM.s
    PRE_TARGETDEPS += cdvd.s
    PRE_TARGETDEPS += usbd.s
    PRE_TARGETDEPS += usbhdfsd.s

    #SOURCES += ports/ps2/cdvd_rpc.c
    QMAKE_CXX = $${PS2DEV}/ee/bin/mips64r5900el-ps2-elf-g++
    #QMAKE_LINK = ee-g++
    QMAKE_LINK = $${PS2DEV}/ee/bin/mips64r5900el-ps2-elf-g++
    #QMAKE_CXXFLAGS += -std=c++11 -G0 -DPLAYSTATION2=1 -DNUM=100 -DUSE_RWOPS -fpermissive -I../include -I./include -I$${PS2SDK}/ports/include/SDL -I$${PS2SDK}/ports/include -I../common -I$${PS2SDK}/ee/include -I$${PS2SDK}/common/include -w
    QMAKE_CXXFLAGS += -DPLAYSTATION2=1 -D_EE -G0 -O2 -Wall \
    -I$${PS2SDK}/ports/include \
    -I$${PS2SDK}/ee/include \
    -I$${PS2SDK}/common/include \
    -I. -I../include \
    -I$${PS2SDK}/common/include -w

    QMAKE_CFLAGS -= -m64
#    QMAKE_CFLAGS -= -std=gnu++11
    QMAKE_CXXFLAGS -= -m64
#    QMAKE_CXXFLAGS -= -std=gnu++11
    QMAKE_LFLAGS -= -m64

    INCLUDES = -D_EE -O3 -G0 -Wall -G0 -mno-check-zero-division -ffast-math -funroll-loops -fomit-frame-pointer \
    -fstrict-aliasing -funsigned-char -fno-builtin-printf  -I. -Iunzip -DVAR_CYCLES -DCPU_SHUTDOWN -DSPC700_SHUTDOWN \
    -DEXECUTE_SUPERFX_PER_LINE   -DSPC700_C  -DUNZIP_SUPPORT    -DSDD1_DECOMP  -DNO_INLINE_SET_GET -DNOASM -D_STLP_NO_NAMESPACES \
    -D_NOTHREADS -D_STLP_NO_EXCEPTIONS -D_STLP_USE_NEWALLOC -D_STLP_HAS_WCHAR_T -D_STLP_NO_IOSTREAMS -Dwint_t=int \
    -DPLAYSTATION2=1 -DNUM=100 -DUSE_RWOPS -I../include -I./include -I$${PS2SDK}/ports/include/SDL -I$${PS2SDK}/ports/include -I../common I$${PS2SDK}/ee/include

    LIBS = -Dmain=SDL_main $(SUBLIBS) ../ports/ps2/cdvd.s ../ports/ps2/usbd.s ../ports/ps2/usbhdfsd.s ../ports/ps2/SJPCM.s -L. -lstdc++ -lc \
    -L$(PS2DEV)/gsKit/lib -L../lib -L$${PS2SDK}/ports/lib \
    -lSDL_gfx -lSDL_image -ljpeg -ltiff -lpng -lz -ldebug -lSDL_ttf -lsdlmixer -lSDLmain -lSDL -lfreetype -lm -lcdvd -lpacket -ldma \
    -L$${PS2DEV}/ps2sdk/ee/lib -L$${PS2DEV}/gsKit/lib -L$${PS2DEV}/ps2sdk/ports/lib -lmc \
    -lstdc++  -Wl,--whole-archive -Wl,--no-whole-archive -Wl,--whole-archive -lkernel -Wl,--no-whole-archive
    #QMAKE_POST_LINK += ee-strip --strip-all rockbot.elf
}

playstation3 {
    TARGET = rockbot
    PS3SDK = /usr/local/ps3dev
    PSL1GHT = /usr/local/ps3dev
    export(PSL1GHT)

    QMAKE_CXX = $${PS3SDK}/ppu/bin/ppu-g++
    QMAKE_LINK = $${PS3SDK}/ppu/bin/ppu-g++

    QMAKE_CXXFLAGS += -pipe -g -Wall -W -D_REENTRANT -DPS3 -I$${PS3SDK}/portlibs/ppu/include

    LIBS = $(SUBLIBS) -L$${PS3SDK}/portlibs/ppu/lib -L/usr/local/ps3dev/portlibs/ppu/lib -lSDL_gfx -lSDL_image -lpng -ltiff -ljpeg -lz -lSDL_ttf -lfreetype -lSDL_mixer -lvorbisfile -lvorbis -logg -lmikmod -lmad `/usr/local/ps3dev/portlibs/ppu/bin/sdl-config --libs`


    INCLUDES = -I. -I../include -I.

    #QMAKE_POST_LINK += $(PANDORASDK)/bin/arm-angstrom-linux-gnueabi-strip --strip-all rockbot_pandora
}

psp {
    TARGET = rockbot_psp.elf
    PSPSDK = /media/iuri/SamsungEXT4/development/SDK/PSP/sdk/

    DEFINES += PSP
    DEFINES += _PSP_FW_VERSION=150

    QMAKE_CC = $${PSPSDK}/bin/psp-gcc
    QMAKE_CXX = $${PSPSDK}/bin/psp-g++
    QMAKE_LINK = $${PSPSDK}/bin/psp-g++

    QMAKE_CFLAGS -= -DQT_QML_DEBUG
    QMAKE_CXXFLAGS -= -DQT_QML_DEBUG

    QMAKE_CFLAGS -= -pipe
    QMAKE_CXXFLAGS -= -pipe

    QMAKE_CFLAGS -= -W
    QMAKE_CXXFLAGS -= -W


    QMAKE_CFLAGS += -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I. -I/psp/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk//include -G0 -G0 -Wall -O3 -DPSP -DHANDHELD -g -fexceptions -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I. -I/psp/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk//include -G0 -G0 -Wall -O3 -DPSP -DHANDHELD -g -fexceptions -fno-exceptions -fno-rtti -g -fexceptions -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/sdk/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/SDL/ -D_PSP_FW_VERSION=150   -c -o
    QMAKE_CXXFLAGS += -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I. -I/psp/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk//include -G0 -G0 -Wall -O3 -DPSP -DHANDHELD -g -fexceptions -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I. -I/psp/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk//include -G0 -G0 -Wall -O3 -DPSP -DHANDHELD -g -fexceptions -fno-exceptions -fno-rtti -g -fexceptions -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/sdk/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/SDL/ -D_PSP_FW_VERSION=150   -c -o

    LIBS = -lm -lc -lstdc++ -lc -lSDL_gfx -lSDL_mixer -logg -lmikmod -lsmpeg -lSDL_ttf -lfreetype -lSDL_image -ljpeg -lpng -lz \
    -lSDLmain -lSDL -lm -lGL -lm -L/usr/local/pspdev/psp/sdk/lib -L/usr/local/pspdev/psp -lpspirkeyb -lpsppower -lpspvfpu -lpspdebug -lpspgu -lpspctrl -lpspge -lpspdisplay -lpsphprm -lpspaudio \
    -L$(PSPSDK)/sdk/lib -L/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/sdk/lib

    INCLUDES = -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/sdk/include -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/ -I/media/iuri/SamsungEXT4/development/SDK/PSP/sdk/psp/include/SDL/

    QMAKE_POST_LINK += /media/iuri/SamsungEXT4/development/SDK/PSP/sdk/bin/mksfo 'Rockbot ' PARAM.SFO $$escape_expand(\n\t)
    QMAKE_POST_LINK += /media/iuri/SamsungEXT4/development/SDK/PSP/sdk/bin/psp-strip rockbot_psp.elf -o rockbot_psp_strip.elf $$escape_expand(\n\t)
    QMAKE_POST_LINK += /media/iuri/SamsungEXT4/development/SDK/PSP/sdk/bin/pack-pbp EBOOT.PBP PARAM.SFO ./packages/files/psp_icon.png NULL NULL ./packages/files/psp_background.png NULL rockbot_psp_strip.elf NULL $$escape_expand(\n\t)

#mksfo 'Rockbot ' PARAM.SFO
#psp-strip rockbot_psp.elf -o rockbot_psp_strip.elf
#pack-pbp EBOOT.PBP PARAM.SFO ./build/packages/files/psp_icon.png  \
#        NULL NULL ./build/packages/files/psp_background.png  \
#        NULL  rockbot_psp_strip.elf NULL
#
#[0]        408 bytes | PARAM.SFO
#[1]       7146 bytes | ./build/packages/files/psp_icon.png
#[2]          0 bytes | NULL
#[3]          0 bytes | NULL
#[4]     134559 bytes | ./build/packages/files/psp_background.png
#[5]          0 bytes | NULL
#[6]    3783804 bytes | rockbot_psp_strip.elf
#[7]          0 bytes | NULL


}



open_pandora {
    TARGET = rockbot_pandora
    PANDORASDK = /media/iuri/SamsungEXT4/development/SDK/pandora_sdk/arm-2011.09/

    QMAKE_CXX = $${PANDORASDK}/bin/arm-angstrom-linux-gnueabi-g++
    QMAKE_LINK = $${PANDORASDK}/bin/arm-angstrom-linux-gnueabi-g++

    QMAKE_CXXFLAGS += -pipe -g -Wall -W -D_REENTRANT -DOPEN_PANDORA -DHANDHELD -I$(PANDORAROOTDIR)/usr/include

    LIBS = $(SUBLIBS) -L$(PANDORAROOTDIR)/lib -L$(PANDORAROOTDIR)/usr/lib -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx `$(PANDORAROOTDIR)/usr/bin/sdl-config --libs` -lpthread

    INCLUDES = -I. -I../include -I.

    #QMAKE_POST_LINK += $(PANDORASDK)/bin/arm-angstrom-linux-gnueabi-strip --strip-all rockbot_pandora
}


wii {
    TARGET = rockbot.elf
    QMAKE_CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc
    QMAKE_CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++
    QMAKE_LINK = $(DEVKITPPC)/bin/powerpc-eabi-g++

    QMAKE_CXXFLAGS += -Dmain=SDL_main -fexceptions -G0 -Wall -O3 -DWII -DHANDHELD -g -I. -I$(DEVKITPPC)/../libogc/include/  -I$(DEVKITPPC)/../libogc/include/ogc/ -I$(DEVKITPPC)/devkitPPC/include/ -G0 -Wall -DWII -g -fno-rtti -g
    LIBS = -Dmain=SDL_main -L. -L$(DEVKITPPC)/../portlibs/ppc/lib -L$(DEVKITPPC)/../libogc/lib/wii/ -L$(DEVKITPPC)/../libogc/lib/ -L$(DEVKITPPC)/devkitPPC/lib/ -lSDL_gfx -lSDL_ttf -lSDL_mixer -lSDL_image -lsmpeg -lSDL -ljpeg -lpng -lfreetype -lz -lfat -lwiiuse -lbte -lwiikeyboard -logc -lm -mrvl

    INCLUDES = -I$(DEVKITPPC)/libogc/include/ -I$(DEVKITPPC)/devkitPPC/include/ -I. -I../include -I.

    QMAKE_POST_LINK += $(DEVKITPPC)/bin/elf2dol rockbot.elf boot.dol
}

dreamcast {
    TARGET = rockbot_dreamcast.elf
    QMAKE_CC = /opt/toolchains/dc/sh-elf/bin/sh-elf-gcc
    QMAKE_CXX = /opt/toolchains/dc/sh-elf/bin/sh-elf-g++
    QMAKE_LINK = /opt/toolchains/dc/sh-elf/bin/sh-elf-g++

    QMAKE_CXXFLAGS += -I$(KOS_BASE)/addons/include $(OPTFLAGS) $(KOS_LDFLAGS) -Wall -O3 -DHANDHELD -g -I. -I/home/iuri/devel/SDK/Dreamcast/kos/utils/dc-chain/newlib-2.0.0/newlib/libc/include -I/opt/toolchains/dc/sh-elf/include -DDREAMCAST $(OBJEXTRA) $(KOS_LIBS)
    LIBS = $(KOS_LIBS) $(KOS_LDFLAGS) -L. -L/home/iuri/devel/SDK/Dreamcast/kos/addons/lib/dreamcast/ -lc -lstdc++ -lc -lSDL_gfx -lSDL_ttf -lSDL_mixer_126 -lSDL_image_124 -lSDL_1213 -ljpeg -lpng -lfreetype -lz_123 $(OBJEXTRA) $(KOS_LIBS) -lm -lgcc -lc

    INCLUDES = -I/home/iuri/devel/SDK/Dreamcast/kos-ports/include -I$(KOS_BASE)/addons/include -I$(KOS_BASE)/addons/include/SDL $(OPTFLAGS) -I/opt/toolchains/dc/sh-elf/include -I. -I../include -I.
    #$KOS_OBJCOPY -R .stack -O binary rockbot_dreamcast.elf rockbot_dreamcast.bin
}


QMAKE_CXXFLAGS += $${DEFINESLIST}

TEMPLATE = app


SOURCES += main.cpp \
    aux_tools/fps_control.cpp \
    character/character.cpp \
    data/datautil.cpp \
    graphicslib.cpp \
    inputlib.cpp \
    game.cpp \
    logger.cpp \
    shareddata.cpp \
    stage.cpp \
    classmap.cpp \
    character/classplayer.cpp \
    timerlib.cpp \
    sceneslib.cpp \
    soundlib.cpp \
    projectilelib.cpp \
    character/classnpc.cpp \
    stage_select.cpp \
    class_config.cpp \
    objects/object.cpp \
    graphic/option_picker.cpp \
    scenes/dialogs.cpp \
    file/file_io.cpp \
    graphic/animation.cpp \
    character/artificial_inteligence.cpp \
    options/key_map.cpp \
    graphic/draw.cpp \
    aux_tools/trajectory_parabola.cpp \
    file/convert.cpp \
    character/movement/jump.cpp \
    character/movement/inertia.cpp \
    file/fio_scenes.cpp \
    scenes/sceneshow.cpp \
    strings_map.cpp \
    file/fio_strings.cpp \
    aux_tools/stringutils.cpp \
    file/fio_common.cpp \
    game_mediator.cpp \
    collision_detection.cpp \
    graphic/gfx_sin_wave.cpp \
    character/character_animation.cpp \
    aux_tools/exception_manager.cpp \
    scenes/game_menu.cpp \
    ports/android/android_game_services.cpp \
    sdl_layer.cpp


HEADERS += \
    aux_tools/fps_control.h \
    character/character.h \
    data/datautil.h \
    file/v4/file_config_old.h \
    graphicslib.h \
    defines.h \
    inputlib.h \
    game.h \
    logger.h \
    shareddata.h \
    stage.h \
    classmap.h \
    character/classplayer.h \
    timerlib.h \
    sceneslib.h \
    soundlib.h \
    projectilelib.h \
    character/classnpc.h \
    stage_select.h \
    class_config.h \
    objects/object.h \
    graphic/option_picker.h \
    scenes/dialogs.h \
    file/format.h \
    file/file_io.h \
    graphic/animation.h \
    character/artificial_inteligence.h \
    character/st_spriteFrame.h \
    ports/ps2/modules.h \
    options/key_map.h \
    graphic/draw.h \
    aux_tools/trajectory_parabola.h \
    file/convert.h \
    file/format/st_characterState.h \
    file/format/st_common.h \
    file/format/st_hitPoints.h \
    file/format/st_platform.h \
    file/format/st_projectile.h \
    file/format/st_teleporter.h \
    character/movement/jump.h \
    character/movement/inertia.h \
    file/version.h \
    file/fio_scenes.h \
    scenes/sceneshow.h \
    file/v4/file_config_v4.h \
    file/v4/file_game_v4.h \
    file/v4/file_save_v4.h \
    file/v4/file_scene_v4.h \
    file/v4/file_stage_v4.h \
    strings_map.h \
    file/fio_strings.h \
    aux_tools/stringutils.h \
    file/v4/file_strings.h \
    file/v4/file_map.h \
    file/fio_common.h \
    game_mediator.h \
    file/v4/file_anim_block.h \
    collision_detection.h \
    graphic/gfx_sin_wave.h \
    character/character_animation.h \
    aux_tools/exception_manager.h \
    scenes/game_menu.h \
    ports/android/android_game_services.h \
    sdl_layer.h

OTHER_FILES += \
    docs/RoadMap.txt \
    rockbot_dingux.pro \
    docs/Changelog.txt \
    docs/graphic_conversion.txt

DISTFILES += \
    AndroidAppSettings.cfg \
    docs/notes.txt \
    docs/Releases.txt \
    docs/crashes.txt \
    docs/translation_lines.txt



