/* -----------------------------------------------------------------------
 * src/msdos/conio.c  --  MS-DOS console I/O helpers.
 *
 * Implements the functions declared in include/msdos/conio.h using
 * Open Watcom's <graph.h> for screen control and the runtime getch()/
 * kbhit() for keyboard input.
 *
 * Also provides get_line() for MS-DOS (get_line.c is cc65-only).
 * ----------------------------------------------------------------------- */

#include <graph.h>      /* _clearscreen, _displaycursor,
                           _GCLEARSCREEN, _GCURSORON, _GCURSOROFF */
#include <i86.h>        /* union REGS, int86 – for BIOS cursor positioning */
#include <dos.h>        /* MK_FP – far pointer construction */
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* getch() and kbhit() live in Open Watcom's C runtime but are NOT in our
   custom conio.h, so forward-declare them here.                          */
extern int getch(void);
extern int kbhit(void);

#include <conio.h>      /* our include/msdos/conio.h: prototypes + chardefs */
#include <screen.h>     /* SCREEN_WIDTH */
#include <get_line.h>

/**
 * @brief Move the cursor to the given 0-based (col, row) position.
 *
 * Uses BIOS INT 10h / AH=02h directly to avoid ambiguity with
 * _settextposition()'s row/col argument order.
 *
 * @param x Column (0-based).
 * @param y Row (0-based).
 */
void gotoxy(int x, int y)
{
    static union REGS r;
    r.h.ah = 0x02;              /* Set Cursor Position */
    r.h.bh = 0x00;              /* video page 0        */
    r.h.dh = (uint8_t)y;  /* row  (0-based)      */
    r.h.dl = (uint8_t)x;  /* col  (0-based)      */
    int86(0x10, &r, &r);
}

/* -----------------------------------------------------------------------
 * Video attribute constants and screen fill helper.
 *
 * Attribute bytes: 0x17 = white-on-blue (normal), 0x70 = black-on-white.
 * These must be defined before clrscr() which calls fill_screen().
 * ----------------------------------------------------------------------- */
#define ATTR_NORMAL  0x17   /* white on blue  */
#define ATTR_REVERSE 0x70   /* black on white */

static uint8_t saved_attr = 0x07;  /* restored in cleanup() */

/**
 * @brief Fill the entire 40-column text screen with spaces using attr.
 *
 * Uses BIOS INT 10h / AH=06h (scroll-up / clear window) so both character
 * and attribute bytes are set correctly — unlike _clearscreen() which
 * resets to the default black background.
 *
 * @param attr Video attribute byte to apply to all cells.
 */
static void fill_screen(uint8_t attr)
{
    static union REGS r;
    r.h.ah = 0x06;
    r.h.al = 0x00;              /* clear (scroll 0 lines = erase all) */
    r.h.bh = attr;              /* fill attribute                      */
    r.h.ch = 0;                 /* upper-left row                      */
    r.h.cl = 0;                 /* upper-left col                      */
    r.h.dh = SCREEN_ROWS - 1;  /* lower-right row                     */
    r.h.dl = SCREEN_WIDTH - 1; /* lower-right col                     */
    int86(0x10, &r, &r);
    gotoxy(0, 0);
}

/**
 * @brief Clear the entire text screen using the normal (white-on-blue) attribute.
 */
void clrscr(void)
{
    fill_screen(ATTR_NORMAL);
}

/**
 * @brief Read the next keypress, translating extended keys to ARROW_* constants.
 *
 * Returns 0 if no key is waiting.  Extended two-byte BIOS sequences (arrows)
 * are collapsed to the ARROW_* constants from include/msdos/chardefs.h.
 *
 * @return Character code, an ARROW_* constant, or 0 if no key is available.
 */
char cgetc(void)
{
    int c;

    if (!kbhit())
        return 0;

    c = getch();
    if (c == 0) {           /* extended key: second byte is the scan code */
        c = getch();
        switch (c) {
            case 0x48: return ARROW_UP;
            case 0x50: return ARROW_DOWN;
            case 0x4B: return ARROW_LEFT;
            case 0x4D: return ARROW_RIGHT;
            default:   return 0;    /* unknown extended key */
        }
    }
    return (char)c;
}

/**
 * @brief Set the 40- or 80-column text video mode.
 *
 * @param mode VIDEO_MODE_40COL or VIDEO_MODE_80COL.
 */
void set_video_mode(VideoMode mode)
{
    _setvideomode(mode == VIDEO_MODE_40COL ? _TEXTC40 : _TEXTC80);
}

/**
 * @brief Save the current screen attribute and fill the screen with white-on-blue.
 *
 * The saved attribute is restored by restore_screen_bg() on exit.
 */
void set_screen_bg_blue(void)
{
    uint8_t far *vbuf = (uint8_t far *)MK_FP(0xB800, 0);
    saved_attr = vbuf[1];   /* save current attribute of first cell */
    fill_screen(ATTR_NORMAL);
}

/**
 * @brief Restore the screen attribute saved by set_screen_bg_blue().
 */
void restore_screen_bg(void)
{
    fill_screen(saved_attr);
}

static int           revers_start_x = 0;
static int           revers_start_y = 0;
static int           in_revers       = 0;
static uint8_t pending_attr    = ATTR_NORMAL;

/**
 * @brief Read the current hardware cursor position via BIOS.
 *
 * @param x Receives the current column (0-based).
 * @param y Receives the current row (0-based).
 */
static void get_cursor(int *x, int *y)
{
    static union REGS r;
    r.h.ah = 0x03;
    r.h.bh = 0x00;
    int86(0x10, &r, &r);
    *x = r.h.dl;
    *y = r.h.dh;
}

/**
 * @brief Stamp a video attribute byte onto a rectangular region of the text buffer.
 *
 * Writes directly to the CGA/VGA text-mode buffer at B800:0000.
 * The region is specified as two cursor positions that define a linear
 * run of cells (not necessarily on the same row).
 *
 * @param x1   Starting column.
 * @param y1   Starting row.
 * @param x2   Ending column (exclusive).
 * @param y2   Ending row (exclusive).
 * @param attr Attribute byte to write.
 */
static void stamp_attr(int x1, int y1, int x2, int y2, uint8_t attr)
{
    uint8_t far *vbuf = (uint8_t far *)MK_FP(0xB800, 0);
    int start = y1 * SCREEN_WIDTH + x1;
    int end   = y2 * SCREEN_WIDTH + x2;
    int pos;
    for (pos = start; pos < end; pos++)
        vbuf[pos * 2 + 1] = attr;
}

/*
 * revers() uses a two-phase open/close model so that both highlighted
 * and normal regions get their attribute bytes explicitly stamped:
 *
 *   revers(highlighted);   <- OPEN: record cursor, set pending attr
 *   printf(...);           <- write text (DOS uses attr 0x07 by default)
 *   revers(0);             <- CLOSE: stamp pending attr on that region
 *
 * This ensures un-highlighting actively restores ATTR_NORMAL (0x07)
 * over cells that may still carry a stale ATTR_REVERSE (0x70).
 */
/**
 * @brief Enable or disable inverse video using a two-phase open/close model.
 *
 * On open (first call), records the cursor position and the desired attribute.
 * On close (second call), stamps that attribute onto all cells written between
 * the two calls by writing directly to the VGA text buffer at B800:0000.
 *
 * @param on 1 to request reverse video, 0 to request normal video.
 */
void revers(uint8_t on)
{
    static int cx, cy;
    if (!in_revers) {
        /* Open: record start position and desired attribute */
        get_cursor(&revers_start_x, &revers_start_y);
        pending_attr = on ? ATTR_REVERSE : ATTR_NORMAL;
        in_revers    = 1;
    } else {
        /* Close: stamp the pending attribute then reset */
        get_cursor(&cx, &cy);
        stamp_attr(revers_start_x, revers_start_y, cx, cy, pending_attr);
        in_revers = 0;
    }
}

/**
 * @brief Show or hide the text cursor.
 *
 * @param on true to show, false to hide.
 */
void cursor(bool on)
{
    _displaycursor(on ? _GCURSORON : _GCURSOROFF);
}

/**
 * @brief Read a line of up to (max_len-1) characters with echo and editing.
 *
 * Echoes printable characters, handles backspace (0x08), and stops on
 * ENTER (0x0D) or BREAK (ESC, 0x1B).  Returns an empty string on BREAK.
 *
 * @param buf     Output buffer for the entered string.
 * @param max_len Maximum number of characters (including NUL terminator).
 */
void get_line(char *buf, uint8_t max_len)
{
    uint8_t c;
    uint8_t i = 0;

    buf[0] = '\0';

    do {
        /* cgetc() is non-blocking; spin until a key arrives */
        while ((c = (uint8_t)cgetc()) == 0)
            ;

        if (c == (uint8_t)BREAK) {
            buf[0] = '\0';
            return;
        } else if (c == '\b') {
            if (i > 0) {
                --i;
                buf[i] = '\0';
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else if (isprint(c) && i < (uint8_t)(max_len - 1)) {
            buf[i++] = (char)c;
            putchar(c);
        }
    } while (c != (uint8_t)ENTER);

    buf[i] = '\0';
    putchar('\n');
}

/**
 * @brief Report whether the platform clears the screen on exit.
 *
 * Returns 0 because DOS leaves the screen as-is; cleanup() must call
 * clrscr() explicitly before exiting.
 *
 * @return Always 0.
 */
uint8_t doesclrscrafterexit(void)
{
    return 0;
}
