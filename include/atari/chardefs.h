#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  Atari 8-bit key code definitions.
 *
 * Values from cc65 atari.h (CH_CURS_* / CH_ENTER / CH_ESC).
 * ENTER is ATASCII 0x9B (the Atari end-of-line code), not 0x0D.
 * This file is shared by both the atari and atarixe platform builds.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x1C  /* ATASCII cursor up    (cc65 CH_CURS_UP)    */
#define ARROW_DOWN  0x1D  /* ATASCII cursor down  (cc65 CH_CURS_DOWN)  */
#define ARROW_LEFT  0x1E  /* ATASCII cursor left  (cc65 CH_CURS_LEFT)  */
#define ARROW_RIGHT 0x1F  /* ATASCII cursor right (cc65 CH_CURS_RIGHT) */
#define ENTER       0x9B  /* ATASCII EOL / RETURN (cc65 CH_ENTER)      */
#define BREAK       0x1B  /* ESC                  (cc65 CH_ESC)        */
#define NEWLINE     0x9B

#endif /* CHARDEFS_H */
