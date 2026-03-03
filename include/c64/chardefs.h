#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  Commodore 64 key code definitions.
 *
 * Values from cc65 cbm.h (CH_CURS_* / CH_ENTER / CH_ESC).
 * Cursor keys use PETSCII: up/left are the shifted (high-bit) forms of
 * down/right.  The C64 has no ESC key; 0x1B is mapped here per
 * project convention — consider CH_STOP (0x03, RUN/STOP) if ESC is
 * unavailable on the target keyboard.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x91  /* PETSCII cursor up    (cc65 CH_CURS_UP)    */
#define ARROW_DOWN  0x11  /* PETSCII cursor down  (cc65 CH_CURS_DOWN)  */
#define ARROW_LEFT  0x9D  /* PETSCII cursor left  (cc65 CH_CURS_LEFT)  */
#define ARROW_RIGHT 0x1D  /* PETSCII cursor right (cc65 CH_CURS_RIGHT) */
#define ENTER       0x0D  /* CR                   (cc65 CH_ENTER)      */
#define BREAK       0x1B  /* ESC                  (cc65 CH_ESC)        */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
