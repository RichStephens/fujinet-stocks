#include <fujinet-fuji.h>
#include <ctype.h>
#include <conio.h>


#define SCREEN_BUFFER (byte*) 0xA00

byte colorset = 0;
uint8_t inverse_mode = 0;

/**
 * @brief Initialise the 42-column hi-res text screen via the hirestxt library.
 *
 * Configures PMODE 4 graphics for the screen buffer, redirects printf() to
 * the 42x24 text surface, and wires up the 60 Hz TIMER counter.
 */
void hirestxt_init(void)
{
    // Define a `HiResTextScreenInit` object:
    struct HiResTextScreenInit init =
        {
            42,                 /* characters per row */
            writeCharAt_42cols, /* must be consistent with previous field */
            SCREEN_BUFFER,      /* pointer to the text screen buffer */
            TRUE,               /* redirects printf() to the 42x24 text screen */
            (word *)0x112,      /* pointer to a 60 Hz async counter (Color Basic's TIMER) */
            0,                  /* default cursor blinking rate */
            NULL,               /* use inkey(), i.e., Color Basic's INKEY$ */
            NULL,               /* no sound on '\a' */
        };

    width(32);                               /* PMODE graphics will only appear from 32x16 (does nothing on CoCo 1&2) */
    pmode(4, (byte *)init.textScreenBuffer); /* hires text mode */
    pcls(255);
    screen(1, colorset);
    initHiResTextScreen(&init);
}

/**
 * @brief Shut down the hi-res text screen and restore normal graphics mode.
 */
void hirestxt_close(void)
{
    closeHiResTextScreen();
    width(32);
    pmode(0, 0);
    screen(0, 0);
    clrscr();
}

/**
 * @brief Toggle between the two CoCo hi-res text color sets.
 *
 * colorset 0 is the default; colorset 1 is the alternate palette.
 */
void switch_colorset(void)
{
    if (colorset == 0)
    {
        colorset = 1;
    }
    else
    {
        colorset = 0;
    }

    screen(1, colorset);
}

/**
 * @brief Move the cursor to the given 0-based (col, row) position.
 *
 * @param x Column (0-based).
 * @param y Row (0-based).
 */
void gotoxy(int x, int y)
{
    moveCursor((byte) x, (byte) y);
}

/**
 * @brief Return the current cursor column.
 *
 * @return 0-based column index.
 */
int wherex(void)
{
    return (int) getCursorColumn();
}

/**
 * @brief Return the current cursor row.
 *
 * @return 0-based row index.
 */
int wherey(void)
{
    return (int) getCursorRow();
}

/**
 * @brief Show or hide the blinking cursor.
 *
 * @param onoff true to enable, false to disable.
 */
void cursor(bool onoff)
{
    if (onoff)
        animateCursor();
    else
        removeCursor();
}

byte cgetc_cursor()
{
  byte shift = false;
  byte k;

  while (true)
  {
    k = waitKeyBlinkingCursor();

    if (isKeyPressed(KEY_PROBE_SHIFT, KEY_BIT_SHIFT))
    {
      shift = 0x00;
    }
    else
    {
      if (k > '@' && k < '[')
        shift = 0x20;
    }

    if (k)
      return k + shift;
  }
}


/**
 * @brief Read a line of input with echo and basic editing support.
 *
 * Handles printable characters, left-arrow as backspace, BREAK to cancel
 * (returns empty string), and ENTER to confirm.
 *
 * @param buf     Output buffer for the entered string.
 * @param max_len Maximum number of characters (including NUL terminator).
 */
void get_line(char *buf, uint8_t max_len)
{
    uint8_t c;
    uint8_t i = 0;

    do
    {
        c = cgetc_cursor();
        if (c == BREAK)
        {
            buf[0] = '\0';
            cursor(false);
            return;
        }
        else if (isprint(c))
        {
            putchar(c);
            buf[i] = c;
            if (i < max_len - 1)
                i++;
        }
        else if (c == ARROW_LEFT)
        {
            if (i)
            {
                putchar(ARROW_LEFT);
                putchar(' ');
                putchar(ARROW_LEFT);
                --i;
            }
        }
    } while (c != ENTER);
    putchar('\n');
    buf[i] = '\0';
}

/**
 * @brief Enable or disable inverse video mode.
 *
 * @param on 1 to enable inverse, 0 to disable.
 */
void revers(uint8_t on)
{
    inverse_mode = on;
    setInverseVideoMode((byte)on);
}

/**
 * @brief Block until the 60 Hz TIMER advances by one tick (~16.7 ms).
 *
 * Reads Color Basic's two-byte counter at 0x0112 via getTimer().
 */
void sync_frame(void)
{
    word t = getTimer();
    while (getTimer() == t)
        ;
}

/**
 * @brief Report whether the platform clears the screen on exit.
 *
 * Returns 1 because hirestxt_close() clears the screen and coldStart()
 * never returns, so no explicit clrscr() is needed in cleanup().
 *
 * @return Always 1.
 */
uint8_t doesclrscrafterexit(void)
{
    return 1;
}
