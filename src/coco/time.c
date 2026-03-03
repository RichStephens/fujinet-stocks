/* -----------------------------------------------------------------------
 * time.c  –  CoCo clock() implementation.
 *
 * Wraps getTimer() (from coco.h / fujinet-fuji) into the cc65-compatible
 * clock() interface.  getTimer() increments at 60 Hz, matching
 * CLOCKS_PER_SEC in include/coco/time.h.
 * ----------------------------------------------------------------------- */

#include <time.h>
#include <coco.h>

clock_t clock(void)
{
    return (clock_t)getTimer();
}
