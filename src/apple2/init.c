#include <cc65.h>
#include <conio.h>
#include "init.h"

/**
 * @brief Apple II platform initialization.
 *
 * Sets up platform-dependent resources before the main loop runs.
 */
void init(void)
{
    cursor(0);
}

/**
 * @brief Apple II platform cleanup.
 *
 * Releases platform-dependent resources and restores system state on exit.
 */
void cleanup(void)
{
    cursor(1);
    if (!doesclrscrafterexit())
        clrscr();
}
