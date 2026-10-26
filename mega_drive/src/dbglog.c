#include "dbglog.h"

#if defined(MD_TRANS_DEBUG) || defined(MD_LINK_DEBUG)

#include <stdarg.h>

// The save code uses bytes 0..~130, nothing near 0x4000.
#define DBG_SRAM_BASE 0x4000
#define DBG_SRAM_END  0x7F00
static u16 dbg_pos;

void dbg_log(const char *fmt, ...)
{
    char buf[160];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);
    SRAM_enable();
    for (const char *c = buf; *c && DBG_SRAM_BASE + dbg_pos < DBG_SRAM_END; c++)
        SRAM_writeByte(DBG_SRAM_BASE + dbg_pos++, (u8)*c);
    SRAM_writeByte(DBG_SRAM_BASE + dbg_pos, 0);   // terminator after the last line
}

#endif
