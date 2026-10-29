#include "walkdiag.h"

#ifdef MD_WALK_DIAG

#include "dbgprint.h"
#include "tdump.h"

extern vu16 VBlankProcess;      // sys.c

void wd_ck(const char *tag)
{
    dbg_print("CK %s: vt=%u imask=%u vbp=%04X autoflush=%u dmaq=%u/%u bytes=%u "
              "heap=%u/%u z80bus=%u xgm2=%u\n",
              tag, (unsigned)vtimer, (unsigned)SYS_getInterruptMaskLevel(),
              (unsigned)VBlankProcess, (unsigned)DMA_getAutoFlush(),
              (unsigned)DMA_getQueueSize(), (unsigned)DMA_getMaxQueueSize(),
              (unsigned)DMA_getQueueTransferSize(),
              (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock(),
              (unsigned)Z80_isBusTaken(), (unsigned)XGM2_isPlaying());
    dbg_print("CK %s: regs 01=%02X 02=%02X 03=%02X 04=%02X 05=%02X 07=%02X 0B=%02X "
              "0C=%02X 0D=%02X 0F=%02X 10=%02X 11=%02X 12=%02X | BGA=%04X BGB=%04X "
              "WIN=%04X SPR=%04X HSC=%04X vc=%u\n",
              tag, VDP_getReg(1), VDP_getReg(2), VDP_getReg(3), VDP_getReg(4),
              VDP_getReg(5), VDP_getReg(7), VDP_getReg(11), VDP_getReg(12),
              VDP_getReg(13), VDP_getReg(15), VDP_getReg(16), VDP_getReg(17),
              VDP_getReg(18), VDP_getBGAAddress(), VDP_getBGBAddress(),
              VDP_getWindowAddress(), VDP_getSpriteListAddress(),
              VDP_getHScrollTableAddress(), (unsigned)GET_VCOUNTER);
}

#endif
