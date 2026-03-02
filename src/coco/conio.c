#include <fujinet-fuji.h>
#include <ctype.h>
#include <conio.h>

#ifdef clrscr
#undef clrscr
#endif

#include <hirestxt.h>

#define SCREEN_BUFFER (byte*) 0xA00

byte colorset = 0;
bool cursor_on = false;
unsigned char inverse_mode = 0;

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

void hirestxt_close(void)
{
    closeHiResTextScreen();
    width(32);
    pmode(0, 0);
    screen(0, 0);
    clrscr();
}

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

void gotoxy(int x, int y)
{
    moveCursor((byte) x, (byte) y);
}

int wherex(void)
{
    return (int) getCursorColumn();
}

int wherey(void)
{
    return (int) getCursorRow();
}

void cursor(bool onoff)
{
    if (!cursor_on && onoff)
    {
        animateCursor();
    }
    else if (cursor_on && !onoff)
    {
        removeCursor();
    }

    cursor_on = onoff;
}

void clear_screen(byte color)
{
    clrscr();
}

char cgetc()
{
    byte shift = false;
    byte k;

    while (true)
    {
        if (cursor_on)
        {
            k = waitKeyBlinkingCursor();
        }
        else
        {
            k = inkey();
        }

        if (isKeyPressed(KEY_PROBE_SHIFT, KEY_BIT_SHIFT))
        {
            shift = 0x00;
        }
        else
        {
            if (k > '@' && k < '[')
            {
                shift = 0x20;
            }
        }

        return (char)k + shift;
    }
}

bool get_line(char *buf, int max_len)
{
    uint8_t c;
    uint16_t i = 0;

    do
    {
        cursor(true);
        c = cgetc();
        if (c == BREAK)
        {
            cursor(false);
            return false;
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

    cursor(false);
    return true;
}

/* revers()
 * Enable or disable inverse video mode.
 * on = 1 turns inverse on; on = 0 turns it off.
 * State is tracked in inverse_mode so callers can query it if needed.
 */
void revers(unsigned char on)
{
    inverse_mode = on;
    setInverseVideoMode((byte)on);
}

/* sync_frame()
 * Block until the 60 Hz TIMER advances by one tick.
 * getTimer() is a cmoc/CoCo runtime function that reads Color Basic's
 * two-byte counter at 0x0112.  Each tick is ~16.7 ms.
 */
void sync_frame(void)
{
    word t = getTimer();
    while (getTimer() == t)
        ;
}
