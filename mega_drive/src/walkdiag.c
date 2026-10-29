#include "walkdiag.h"

#ifdef MD_WALK_DIAG

#include "dbgprint.h"
#include "tdump.h"
#include <string.h>

extern vu16 VBlankProcess;      // sys.c
extern DMAOpInfo *dmaQueues;    // dma.c (global: the flush routine reads it)
extern u16 *dmaDataBuffer;


// Does a DMA land RIGHT NOW? A known 8-word pattern is DMA'd into an unused VRAM
// area (the window plane, which is switched off: VRAM 0xD800) and read back, from a
// ROM source and from a RAM source, immediate and (on request) through the queue.
// Printed at every checkpoint, so the log shows the first step after which DMA
// stops working, not just that it does not work at the end.
static const u16 rom_pat[8] = { 0xA5A5, 0x5A5A, 0x1234, 0x4321, 0xBEEF, 0xFEED, 0x0F0F, 0xF0F0 };
static const u32 zero_tile[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#define PROBE_VRAM 0xD800

static bool probe_ok(void)
{
    u16 b[8];
    tdump_read(PROBE_VRAM, b, 8);
    for (u8 i = 0; i < 8; i++) if (b[i] != rom_pat[i]) return FALSE;
    return TRUE;
}

static void probe_clear(void)
{
    VDP_loadTileData(zero_tile, PROBE_VRAM / 32, 1, CPU);
}

static void wd_probe(const char *tag, bool queued)
{
    static u16 ram_pat[8];
    memcpy(ram_pat, rom_pat, 16);
    char r[3][8];
    probe_clear();
    DMA_doDma(DMA_VRAM, (void *)rom_pat, PROBE_VRAM, 8, 2);
    strcpy(r[0], probe_ok() ? "OK" : "FAIL");
    probe_clear();
    DMA_doDma(DMA_VRAM, ram_pat, PROBE_VRAM, 8, 2);
    strcpy(r[1], probe_ok() ? "OK" : "FAIL");
    strcpy(r[2], "-");
    if (queued) {
        probe_clear();
        const u16 before = DMA_getQueueSize();
        const bool queued_ok = DMA_queueDma(DMA_VRAM, (void *)rom_pat, PROBE_VRAM, 8, 2);
        // Is the entry in the queue memory what the library says it wrote? (pointer in
        // RAM? contents equal to the register values for this transfer?)
        const u32 from = (u32)rom_pat;
        const DMAOpInfo *e = &dmaQueues[before];
        const u32 w_len = 0x93000008UL, w_len_h = 0x9400;
        const u32 w_ms = 0x96008F00UL + ((from << 7) & 0xFF0000UL) + 2;
        const u32 w_ha = 0x97009500UL + ((from >> 1) & 0x7F00FFUL);
        const u32 w_ct = VDP_DMA_VRAM_ADDR((u32)PROBE_VRAM);
        const bool entry_ok = ((u16)w_len == e->regLenL) && (w_len_h == e->regLenH) &&
                              (w_ms == e->regAddrMStep) && (w_ha == e->regAddrHAddrL) &&
                              (w_ct == e->regCtrlWrite);
        dbg_print("QCHK %s: dmaQueues=%08X dmaDataBuffer=%08X maxq=%u queued_ok=%u idx=%u "
                  "entry=%s got=%04X %04X %08X %08X %08X want=%04X %04X %08X %08X %08X\n",
                  tag, (unsigned)dmaQueues, (unsigned)dmaDataBuffer,
                  (unsigned)DMA_getMaxQueueSize(), (unsigned)queued_ok, (unsigned)before,
                  entry_ok ? "OK" : "BAD", e->regLenL, e->regLenH, (unsigned)e->regAddrMStep,
                  (unsigned)e->regAddrHAddrL, (unsigned)e->regCtrlWrite, (unsigned)(u16)w_len,
                  (unsigned)w_len_h, (unsigned)w_ms, (unsigned)w_ha, (unsigned)w_ct);
        SYS_doVBlankProcess();
        strcpy(r[2], probe_ok() ? "OK" : "FAIL");
    }
    dbg_print("PROBE %s: dma_rom=%s dma_ram=%s queued_rom=%s\n", tag, r[0], r[1], r[2]);
}

void wd_ckq(const char *tag)
{
    wd_ck(tag);
    wd_probe(tag, TRUE);
}

void wd_ck(const char *tag)
{
    {
        const u16 st = *(vu16 *)VDP_CTRL_PORT;      // VDP status: b9 FIFO empty, b8 full, b3 vblank, b1 DMA busy
        dbg_print("CK %s: status=%04X\n", tag, (unsigned)st);
    }
    dbg_print("CK %s: vt=%u imask=%u vbp=%04X autoflush=%u dmaq=%u/%u bytes=%u "
              "heap=%u/%u z80bus=%u xgm2=%u\n",
              tag, (unsigned)vtimer, (unsigned)SYS_getInterruptMaskLevel(),
              (unsigned)VBlankProcess, (unsigned)DMA_getAutoFlush(),
              (unsigned)DMA_getQueueSize(), (unsigned)DMA_getMaxQueueSize(),
              (unsigned)DMA_getQueueTransferSize(),
              (unsigned)MEM_getFree(), (unsigned)MEM_getLargestFreeBlock(),
              (unsigned)Z80_isBusTaken(), (unsigned)XGM2_isPlaying());
    // Z80 driver view of DMA: how long its PCM waited for the 68k's DMA (the
    // protection handshake) and its CPU load, whether the flush delay option is on,
    // and the DMA limits.
    {
        u32 sp;
        __asm__ volatile("move.l %%sp,%0" : "=r"(sp));
        dbg_print("CK %s: sp=%08X dmaQueues=%08X dmaDataBuffer=%08X\n", tag, (unsigned)sp,
                  (unsigned)dmaQueues, (unsigned)dmaDataBuffer);
    }
    dbg_print("CK %s: xgm2 dmawait=%u/%u cpu=%u/%u forcedelay=%u z80drv=%d "
              "maxxfer=%u ignoreover=%u\n", tag,
              (unsigned)XGM2_getDMAWaitTime(FALSE), (unsigned)XGM2_getDMAWaitTime(TRUE),
              (unsigned)XGM2_getCPULoad(FALSE), (unsigned)XGM2_getCPULoad(TRUE),
              (unsigned)Z80_getForceDelayDMA(), (int)Z80_getLoadedDriver(),
              (unsigned)DMA_getMaxTransferSize(), (unsigned)DMA_getIgnoreOverCapacity());
    dbg_print("CK %s: regs 01=%02X 02=%02X 03=%02X 04=%02X 05=%02X 07=%02X 0B=%02X "
              "0C=%02X 0D=%02X 0F=%02X 10=%02X 11=%02X 12=%02X | BGA=%04X BGB=%04X "
              "WIN=%04X SPR=%04X HSC=%04X vc=%u\n",
              tag, VDP_getReg(1), VDP_getReg(2), VDP_getReg(3), VDP_getReg(4),
              VDP_getReg(5), VDP_getReg(7), VDP_getReg(11), VDP_getReg(12),
              VDP_getReg(13), VDP_getReg(15), VDP_getReg(16), VDP_getReg(17),
              VDP_getReg(18), VDP_getBGAAddress(), VDP_getBGBAddress(),
              VDP_getWindowAddress(), VDP_getSpriteListAddress(),
              VDP_getHScrollTableAddress(), (unsigned)GET_VCOUNTER);
    wd_probe(tag, FALSE);
}


#endif
