#include <conio.h>
#include "init.h"

/**
 * @brief CoCo platform initialization.
 *
 * Sets up the hi-res text screen via hirestxt before the main loop runs.
 */
void init(void)
{
    hirestxt_init();
}

/**
 * @brief CoCo platform cleanup.
 *
 * Closes the hi-res text screen and performs a cold start to return
 * the system to a clean state on exit.
 */
void cleanup(void)
{
    hirestxt_close();
    coldStart();
}
