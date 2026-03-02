#include <fujinet-fuji.h>
#include <string.h>
#include <conio.h>
#include "fujinet-stocks.h"

AdapterConfigExtended ace;

int main(void)
{
#ifdef _CMOC_VERSION_
    hirestxt_init();
#endif

    /* Verify FujiNet is present before proceeding */
    if (!fuji_get_adapter_config_extended(&ace))
        strcpy(ace.fn_version, "FAIL");

    /* Hand off to the main application loop (screen.c).
       main_loop() returns only when the user presses BREAK. */
    main_loop();

#ifdef _CMOC_VERSION_
    hirestxt_close();
    coldStart();
#endif

    return 0;
}