#ifndef CHARDEFS_H
#define CHARDEFS_H

/* -----------------------------------------------------------------------
 * chardefs.h  –  MSX key code definitions.
 *
 * Values returned by the MSX BIOS CHGET routine (0x009F) for cursor keys.
 * This file is shared by both the msxrom and msxdos platform builds.
 * Verify against your MSX toolchain's keyboard header if available.
 * ----------------------------------------------------------------------- */

#define ARROW_UP    0x1E  /* MSX BIOS cursor up    */
#define ARROW_DOWN  0x1F  /* MSX BIOS cursor down  */
#define ARROW_LEFT  0x1D  /* MSX BIOS cursor left  */
#define ARROW_RIGHT 0x1C  /* MSX BIOS cursor right */
#define ENTER       0x0D  /* CR                    */
#define BREAK       0x1B  /* ESC                   */
#define NEWLINE     0x0A

#endif /* CHARDEFS_H */
