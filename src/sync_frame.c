/* -----------------------------------------------------------------------
 * sync_frame.c
 * Non-CoCo implementation of sync_frame() using clock().
 *
 * Blocks until clock() advances by one tick, synchronising the main loop
 * to the platform's CLOCKS_PER_SEC rate (typically 60 Hz on cc65 targets).
 *
 * The CoCo uses getTimer() directly in src/coco/conio.c instead.
 * ----------------------------------------------------------------------- */

#ifndef _CMOC_VERSION_

#include <time.h>

void sync_frame(void)
{
    clock_t t = clock();
    while (clock() == t)
        ;
}

#endif /* !_CMOC_VERSION_ */
