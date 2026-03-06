#include <stdio.h>
#include "init.h"
#include <conio.h>

/**
 * @brief MS-DOS platform initialization.
 *
 * Disables stdout buffering so every printf/putchar call goes straight to
 * the DOS console driver.  Without this, output is flushed in a batch when
 * the buffer fills, after gotoxy has already moved the cursor elsewhere,
 * which scrambles the screen layout.
 */
void init(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    set_video_mode(VIDEO_MODE_40COL);
    set_screen_bg_blue();
}

/**
 * @brief MS-DOS platform cleanup.
 *
 * Releases platform-dependent resources and restores system state on exit.
 */
void cleanup(void)
{
    restore_screen_bg();
    set_video_mode(VIDEO_MODE_80COL);
    if (!doesclrscrafterexit())
        clrscr();
}
