#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  CoCo key code definitions.
 *
 * cgetc() return values for special keys on the Color Computer.
 * The CoCo uses ^ (0x5E) for cursor-up, LF for down, BS for left,
 * TAB for right, and the hardware BRK key produces 0x03.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x5E  /* '^'  –  cursor up                 */
#define ARROW_DOWN  0x0A  /* LF   –  cursor down               */
#define ARROW_LEFT  0x08  /* BS   –  cursor left               */
#define ARROW_RIGHT 0x09  /* TAB  –  cursor right              */
#define ENTER       0x0D  /* CR                                */
#define BREAK       0x03  /* BRK key                           */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
