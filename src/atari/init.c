#include <cc65.h>
#include <conio.h>
#include "init.h"

/**
 * @brief Atari platform initialization.
 *
 * Sets up platform-dependent resources before the main loop runs.
 */
void init(void)
{
}

/**
 * @brief Atari platform cleanup.
 *
 * Releases platform-dependent resources and restores system state on exit.
 */
void cleanup(void)
{
    if (!doesclrscrafterexit())
        clrscr();
}
