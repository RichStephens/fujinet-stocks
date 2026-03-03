#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  Apple II key code definitions.
 *
 * Values from cc65 apple2.h (CH_CURS_* / CH_ENTER / CH_ESC).
 * Up/Down require an enhanced IIe or IIc; on the plain ][+, only
 * left (BS) and right (ctrl-U) are available.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x0B  /* ctrl-K  –  cursor up (IIe/IIc)    */
#define ARROW_DOWN  0x0A  /* ctrl-J  –  cursor down (IIe/IIc)  */
#define ARROW_LEFT  0x08  /* BS      –  cursor left             */
#define ARROW_RIGHT 0x15  /* ctrl-U  –  cursor right            */
#define ENTER       0x0D  /* CR                                 */
#define BREAK       0x1B  /* ESC                                */
#define NEWLINE     0x0A
#define CH_DEL      0x7F

#define CLOCKS_PER_SEC 60

#endif /* CHARDEFS_H */
