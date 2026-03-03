#include <fujinet-fuji.h>
#include <string.h>
#include <conio.h>
#include "stocks.h"
#include "init.h"

AdapterConfigExtended ace;

int main(void)
{
    init();
    load_stocks();
    get_stock_quotes();

    /* Hand off to the main application loop (screen.c).
       main_loop() returns only when the user presses BREAK. */
    main_loop();

    save_stocks();
    cleanup();

    return 0;
}