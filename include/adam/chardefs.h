#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  Coleco Adam key code definitions.
 *
 * The Adam keyboard uses ADAMnet and has dedicated cursor keys.
 * These values are approximate — verify against the Adam SDK or
 * the toolchain's keyboard header.
 * This file is shared by both the adam and adam_cpm platform builds.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x1C  /* Adam cursor up    – verify  */
#define ARROW_DOWN  0x1D  /* Adam cursor down  – verify  */
#define ARROW_LEFT  0x1E  /* Adam cursor left  – verify  */
#define ARROW_RIGHT 0x1F  /* Adam cursor right – verify  */
#define ENTER       0x0D  /* CR                          */
#define BREAK       0x1B  /* ESC                         */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
