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

#define ARROW_UP    0x1C  /* mapped from BIOS scan 0x48 by cgetc() */
#define ARROW_DOWN  0x1D  /* mapped from BIOS scan 0x50 by cgetc() */
#define ARROW_LEFT  0x1E  /* mapped from BIOS scan 0x4B by cgetc() */
#define ARROW_RIGHT 0x1F  /* mapped from BIOS scan 0x4D by cgetc() */
#define ENTER       0x0D  /* CR                                     */
#define BREAK       0x1B  /* ESC                                    */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
