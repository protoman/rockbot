#ifndef SAVE_H
#define SAVE_H

#include <genesis.h>

// 5 save slots in SRAM, like the PC port (SAVE_MAX_SLOT_NUMBER 4 -> 5).
// Layout (word offsets in the SRAM window): slot i magic at
// 8 + i*8, stages mask at +2. Empty slots have magic 0.
#define SAVE_SLOT_COUNT 5
#define SAVE_MAGIC 0x5242       // 'RB'

void save_init(void);

bool save_exists(void);                 // any slot valid
bool save_slot_exists(u8 slot);
u16 save_slot_stages(u8 slot);          // completed-stages mask (0 if empty)
// Owned-weapon mask: PC dataUtil::has_weapon() — weapon n is owned once stage
// n is beaten; the buster (0) is always owned.
u16 save_slot_weapon_mask(u8 slot);
void save_slot_set_stage(u8 slot, u8 stage);
void save_slot_clear(u8 slot);
u8 save_slot_player(u8 slot);          // selected player 0..3 (0 if unset)
void save_slot_set_player(u8 slot, u8 player);

#endif
