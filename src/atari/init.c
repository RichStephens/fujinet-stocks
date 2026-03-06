#include <cc65.h>
#include <conio.h>
#include <atari.h>
#include "init.h"

/**
 * @brief Atari platform initialization.
 *
 * Sets up platform-dependent resources before the main loop runs.
 */
void init(void)
{
    cursor(0);
    OS.crsinh = 1;
}

/**
 * @brief Atari platform cleanup.
 *
 * Releases platform-dependent resources and restores system state on exit.
 */
void cleanup(void)
{
    cursor(1);
    OS.crsinh = 0;
    if (!doesclrscrafterexit())
        clrscr();
}
