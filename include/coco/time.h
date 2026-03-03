#ifndef TIME_H
#define TIME_H

/* -----------------------------------------------------------------------
 * time.h  –  CoCo shim for cc65-compatible clock() interface.
 *
 * getTimer() on the CoCo increments at 60 Hz, matching cc65's convention,
 * so CLOCKS_PER_SEC is 60 here.
 * ----------------------------------------------------------------------- */

typedef unsigned long clock_t;

#define CLOCKS_PER_SEC  60

clock_t clock(void);

#endif /* TIME_H */
