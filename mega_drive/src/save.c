#include "save.h"

// SRAM is declared by the ROM header (src/rom_header.c: "RA", 0xF820,
// 0x00200000-0x0020FFFF). SGDK's SRAM_readWord/writeWord take a byte offset
// into that window (odd-byte SRAM: word N lands at file bytes N..N+1).
//
// Slot record (8 bytes): [magic u16][stages u16][reserved u32]
// Slot i starts at byte 8 + i*8 (leaves the first 8 bytes free).

#define SLOT_BASE   8
#define SLOT_STRIDE 8

static u16 slot_off(u8 slot)
{
    return (u16)(SLOT_BASE + slot * SLOT_STRIDE);
}

void save_init(void)
{
    SRAM_enable();
}

bool save_slot_exists(u8 slot)
{
    if (slot >= SAVE_SLOT_COUNT) return FALSE;
    return SRAM_readWord(slot_off(slot)) == SAVE_MAGIC;
}

u16 save_slot_stages(u8 slot)
{
    if (!save_slot_exists(slot)) return 0;
    return SRAM_readWord(slot_off(slot) + 2);
}

u16 save_slot_weapon_mask(u8 slot)
{
    // PC dataUtil::has_weapon() (data/datautil.cpp:23): weapon n is owned when
    // stage n has been beaten; the buster (0) is always available.
    return (u16)(save_slot_stages(slot) | 1u);
}

void save_slot_set_stage(u8 slot, u8 stage)
{
    if (slot >= SAVE_SLOT_COUNT) return;
    u16 mask = save_slot_stages(slot);
    mask |= (u16)(1u << stage);
    SRAM_writeWord(slot_off(slot) + 2, mask);
    SRAM_writeWord(slot_off(slot), SAVE_MAGIC);
}

u8 save_slot_player(u8 slot)
{
    if (!save_slot_exists(slot)) return 0;
    return (u8)SRAM_readWord(slot_off(slot) + 4);
}

void save_slot_set_player(u8 slot, u8 player)
{
    if (slot >= SAVE_SLOT_COUNT) return;
    SRAM_writeWord(slot_off(slot) + 4, player);
}

void save_slot_clear(u8 slot)
{
    if (slot >= SAVE_SLOT_COUNT) return;
    SRAM_writeWord(slot_off(slot), 0);
    SRAM_writeWord(slot_off(slot) + 2, 0);
}

bool save_exists(void)
{
    for (u8 i = 0; i < SAVE_SLOT_COUNT; i++) {
        if (save_slot_exists(i)) return TRUE;
    }
    return FALSE;
}
