/* -----------------------------------------------------------------------
 * time.c  –  Apple II clock() implementation.
 *
 * Uses the vertical-blanking indicator at $C019 (Apple IIe and later).
 * Bit 7 of RDVBL is set while the beam is in the blanking interval;
 * we count rising edges to get ~60 ticks per second, matching
 * CLOCKS_PER_SEC as defined by cc65's time.h for the apple2 target.
 * ----------------------------------------------------------------------- */

#include <stdint.h>
#include <time.h>

static clock_t           ticks    = 0;
static uint8_t     last_vbl = 0;

/* RDVBL soft-switch: bit 7 set during vertical blanking (IIe+) */
#define RDVBL  (*(volatile uint8_t *)0xC019)

/**
 * @brief Return the current tick count, updated from the VBL soft-switch.
 *
 * Counts rising edges of bit 7 on RDVBL ($C019) to produce ~60 ticks/sec,
 * matching CLOCKS_PER_SEC for the cc65 apple2 target.
 *
 * @return Elapsed tick count since the program started.
 */
clock_t clock(void)
{
    uint8_t vbl = RDVBL & 0x80;
    if (vbl && !last_vbl)
        ticks++;
    last_vbl = vbl;
    return ticks;
}
