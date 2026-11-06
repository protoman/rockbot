#ifndef SRAM_SAFE_H
#define SRAM_SAFE_H

#include <genesis.h>

// SRAM accesses that leave the ROM visible.
//
// The ROM is larger than 2 MB and the header declares SRAM at 0x200000-0x20FFFF.
// While the mapper has SRAM enabled (SRAM_enable(): $A130F1 = 1) that window is
// SRAM, NOT ROM, so everything the linker placed there reads as save bytes -
// today the mapBlocks of stages 3..10, i.e. a screen of one repeated tile in
// SUN-GOD REALM, DARK MINE, CRYSTAL MAZE ... once a save was touched. Keeping SRAM
// enabled for the whole run (the old save_init) is therefore not allowed:
// each access maps it in, does its work and maps it out again, with interrupts
// off so no vblank DMA/handler reads the window while it is SRAM.
//
// Include this AFTER <genesis.h>; it replaces the SRAM_* accessors.
static inline u8 sram_safe_readByte(u32 off)
{
    SYS_disableInts();
    SRAM_enableRO();
    u8 v = SRAM_readByte(off);
    SRAM_disable();
    SYS_enableInts();
    return v;
}

static inline u16 sram_safe_readWord(u32 off)
{
    SYS_disableInts();
    SRAM_enableRO();
    u16 v = SRAM_readWord(off);
    SRAM_disable();
    SYS_enableInts();
    return v;
}

static inline void sram_safe_writeByte(u32 off, u8 val)
{
    SYS_disableInts();
    SRAM_enable();
    SRAM_writeByte(off, val);
    SRAM_disable();
    SYS_enableInts();
}

static inline void sram_safe_writeWord(u32 off, u16 val)
{
    SYS_disableInts();
    SRAM_enable();
    SRAM_writeWord(off, val);
    SRAM_disable();
    SYS_enableInts();
}

#define SRAM_readByte(off)       sram_safe_readByte(off)
#define SRAM_readWord(off)       sram_safe_readWord(off)
#define SRAM_writeByte(off, v)   sram_safe_writeByte((off), (v))
#define SRAM_writeWord(off, v)   sram_safe_writeWord((off), (v))
// Never leave SRAM mapped in: the only legal enable is inside the wrappers above.
#define SRAM_enable()            ((void)0)

#endif
