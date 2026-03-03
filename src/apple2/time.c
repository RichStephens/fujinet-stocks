/* -----------------------------------------------------------------------
 * time.c  –  Apple II clock() implementation.
 *
 * Uses the vertical-blanking indicator at $C019 (Apple IIe and later).
 * Bit 7 of RDVBL is set while the beam is in the blanking interval;
 * we count rising edges to get ~60 ticks per second, matching
 * CLOCKS_PER_SEC as defined by cc65's time.h for the apple2 target.
 * ----------------------------------------------------------------------- */

#include <time.h>

static clock_t           ticks    = 0;
static unsigned char     last_vbl = 0;

/* RDVBL soft-switch: bit 7 set during vertical blanking (IIe+) */
#define RDVBL  (*(volatile unsigned char *)0xC019)

clock_t clock(void)
{
    unsigned char vbl = RDVBL & 0x80;
    if (vbl && !last_vbl)
        ticks++;
    last_vbl = vbl;
    return ticks;
}
