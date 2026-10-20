#include "save.h"

// SRAM is declared by the ROM header (src/rom_header.c: "RA", 0xF820,
// 0x00200000-0x0020FFFF). SGDK's SRAM_readWord/writeWord take a byte offset
// into that window (odd-byte SRAM: word N lands at file bytes N..N+1).
//
// Slot record (8 bytes): [magic u16][stages u16][reserved u32]
// Slot i starts at byte 8 + i*8 (leaves the first 8 bytes free).

#define SLOT_BASE   8
#define SLOT_STRIDE 8

// Progress record, separate from the slot record so existing saves keep
// working: [magic u16][lives][tanks_hp][tanks_wp][tanks_sp][armor][pad...]
// Slot i at byte 88 + i*12 (the slot records end at 47, config uses 48..55).
#define ITEMS_BASE   88
#define ITEMS_STRIDE 12
#define ITEMS_MAGIC  0x4954     // 'IT'

static u16 slot_off(u8 slot)
{
    return (u16)(SLOT_BASE + slot * SLOT_STRIDE);
}

static u16 items_off(u8 slot)
{
    return (u16)(ITEMS_BASE + slot * ITEMS_STRIDE);
}

void save_init(void)
{
    SRAM_enable();

    // ---- TEMPORARY TEST HACK (see bugs.md, BUG-T01) ----------------------
    // Seed slot 1 — only when it is EMPTY, so a real save is never clobbered —
    // with the intro stage cleared, so a test run can go
    // TITLE -> LOAD GAME -> SLOT 1 -> STAGE SELECT instead of replaying the
    // intro stage (and its boss intro) every time. Delete this block when the
    // port is content-complete.
    if (!save_slot_exists(0)) {
        save_slot_clear(0);
        save_slot_set_stage(0, 0);      // sets the magic too
        save_slot_set_player(0, 0);     // ROCKBOT, so LOAD never reads junk
    }
    // ----------------------------------------------------------------------
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
    // PC dataUtil::has_weapon() (data/datautil.cpp:23): weapons 1..8 need the
    // matching stage; COIL (9) needs APE (stage 1) and JET (10) needs TECHNO
    // (stage 6). Castle bits 9..13 do not unlock weapons.
    u16 stages = save_slot_stages(slot);
    u16 mask = 1u;
    for (u8 n = 1; n <= 8; n++)
        if (stages & (1u << n)) mask |= (u16)(1u << n);
    if (stages & (1u << 1)) mask |= 1u << 9;    // COIL FROG
    if (stages & (1u << 6)) mask |= 1u << 10;   // JET EAGLE
    return mask;
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

bool save_slot_read_items(u8 slot, SaveItems *out)
{
    if (slot >= SAVE_SLOT_COUNT || !out) return FALSE;
    u16 off = items_off(slot);
    if (SRAM_readWord(off) != ITEMS_MAGIC) return FALSE;
    out->lives    = SRAM_readByte(off + 2);
    out->tanks_hp = SRAM_readByte(off + 3);
    out->tanks_wp = SRAM_readByte(off + 4);
    out->tanks_sp = SRAM_readByte(off + 5);
    out->armor    = SRAM_readByte(off + 6);
    return TRUE;
}

void save_slot_write_items(u8 slot, const SaveItems *in)
{
    if (slot >= SAVE_SLOT_COUNT || !in) return;
    u16 off = items_off(slot);
    SRAM_writeWord(off, ITEMS_MAGIC);
    SRAM_writeByte(off + 2, in->lives);
    SRAM_writeByte(off + 3, in->tanks_hp);
    SRAM_writeByte(off + 4, in->tanks_wp);
    SRAM_writeByte(off + 5, in->tanks_sp);
    SRAM_writeByte(off + 6, in->armor);
}

void save_slot_clear(u8 slot)
{
    if (slot >= SAVE_SLOT_COUNT) return;
    SRAM_writeWord(slot_off(slot), 0);
    SRAM_writeWord(slot_off(slot) + 2, 0);
    SRAM_writeWord(items_off(slot), 0);
}

bool save_exists(void)
{
    for (u8 i = 0; i < SAVE_SLOT_COUNT; i++) {
        if (save_slot_exists(i)) return TRUE;
    }
    return FALSE;
}
