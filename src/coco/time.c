/* -----------------------------------------------------------------------
 * time.c  –  CoCo clock() implementation.
 *
 * Wraps getTimer() (from coco.h / fujinet-fuji) into the cc65-compatible
 * clock() interface.  getTimer() increments at 60 Hz, matching
 * CLOCKS_PER_SEC in include/coco/time.h.
 * ----------------------------------------------------------------------- */

#include <time.h>
#include <coco.h>

/**
 * @brief Return the current 60 Hz tick count from Color Basic's TIMER.
 *
 * Wraps getTimer() into the cc65-compatible clock() interface.
 * CLOCKS_PER_SEC is 60 for this target.
 *
 * @return Current value of the CoCo hardware timer.
 */
clock_t clock(void)
{
    return (clock_t)getTimer();
}
