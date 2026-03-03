#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  MS-DOS key code definitions.
 *
 * DOS cursor keys are "extended" keys: INT 16h returns 0x00 followed
 * by the BIOS scan code on the next read.  The scan codes below are
 * the second byte returned after the 0x00 prefix.  Verify that the
 * toolchain's cgetc() handles extended keys and returns these values.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x48  /* BIOS scan code: cursor up    */
#define ARROW_DOWN  0x50  /* BIOS scan code: cursor down  */
#define ARROW_LEFT  0x4B  /* BIOS scan code: cursor left  */
#define ARROW_RIGHT 0x4D  /* BIOS scan code: cursor right */
#define ENTER       0x0D  /* CR                           */
#define BREAK       0x1B  /* ESC                          */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
