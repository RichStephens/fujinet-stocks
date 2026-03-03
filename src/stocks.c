/* -----------------------------------------------------------------------
 * stocks.c
 * Global data declarations and utility functions.
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <screen.h>
#include "stocks.h"
#include "appkey.h"
#include "network.h"

Stock stocks[MAX_STOCKS];

/**
 * @brief Pre-computed screen positions for each stock slot.
 *
 * X positions derived from SLOT_LEFT_X / SLOT_RIGHT_X (screen.h), which are
 * computed from SCREEN_WIDTH so the layout centres correctly on both
 * 42-column (CoCo) and 40-column targets.
 *
 * Slots 0-4 fill the left column top-to-bottom; slots 5-9 fill the right
 * column top-to-bottom.  Rows 5, 8, 11, 14, 17 (2 blank rows between each).
 * Vertical padding: 4 blank rows above (rows 1-4) and below (rows 18-21).
 */
const Coord stock_coords[MAX_STOCKS] = {
    { SLOT_LEFT_X,   5 },   /* slot  0  (stock  1) */
    { SLOT_LEFT_X,   8 },   /* slot  1  (stock  2) */
    { SLOT_LEFT_X,  11 },   /* slot  2  (stock  3) */
    { SLOT_LEFT_X,  14 },   /* slot  3  (stock  4) */
    { SLOT_LEFT_X,  17 },   /* slot  4  (stock  5) */
    { SLOT_RIGHT_X,  5 },   /* slot  5  (stock  6) */
    { SLOT_RIGHT_X,  8 },   /* slot  6  (stock  7) */
    { SLOT_RIGHT_X, 11 },   /* slot  7  (stock  8) */
    { SLOT_RIGHT_X, 14 },   /* slot  8  (stock  9) */
    { SLOT_RIGHT_X, 17 }    /* slot  9  (stock 10) */
};

/**
 * @brief Build the ticker-tape string that scrolls across row 0.
 *
 * Format per stock: "SYMBOL $PP.PP +/-CC.CC%    ". Only populated slots are
 * included.  The buffer is a pure circular tape; draw_ticker_row() reads
 * SCREEN_WIDTH chars at scroll_pos using modulo wrap so entries flow
 * seamlessly.  If total content is shorter than SCREEN_WIDTH the buffer is
 * padded with spaces so the window never repeats.
 *
 * @param buf     Output buffer for the scroll string.
 * @param buf_len Size of the output buffer.
 */
/**
 * @brief Convert a price in integer cents to a display string.
 *
 * Examples: 1234 -> "12.34", 100 -> "1.00", 50 -> "0.50"
 *
 * @param cents   Value scaled by 100 (e.g. 1234 represents $12.34).
 * @param out     Output buffer; must be at least out_len bytes.
 * @param out_len Size of the output buffer.
 */
void format_price(long cents, char *out, int out_len)
{
    long dollars = cents / 100L;
    long rem     = cents % 100L;
    if (rem < 0) rem = -rem;
    snprintf(out, (size_t)out_len, "%ld.%02ld", dollars, rem);
}

void build_scroll_string(char *buf, int buf_len)
{
    int  i, j, pos = 0;
    char price_str[12];
    char change_str[12];
    char sign_char;
    char entry[48];
    long abs_change;

    for (i = 0; i < MAX_STOCKS; i++) {
        if (stocks[i].symbol[0] == '\0')
            continue;

        format_price(stocks[i].price, price_str, sizeof(price_str));

        abs_change = stocks[i].change < 0 ? -stocks[i].change : stocks[i].change;
        format_price(abs_change, change_str, sizeof(change_str));

        sign_char = stocks[i].change > 0 ? '+' :
                    stocks[i].change < 0 ? '-' : ' ';

        snprintf(entry, sizeof(entry), "%s $%s %c%s%%    ",
                 stocks[i].symbol, price_str, sign_char, change_str);

        for (j = 0; entry[j] != '\0' && pos < buf_len - 1; j++)
            buf[pos++] = entry[j];
    }

    /* Pad to at least SCREEN_WIDTH so the circular window never repeats */
    while (pos < SCREEN_WIDTH && pos < buf_len - 1)
        buf[pos++] = ' ';

    buf[pos] = '\0';
}

/**
 * @brief Sort the stock list alphabetically, with empty slots at the end.
 *
 * Uses insertion sort (stable, n=10).  Non-empty slots are sorted by
 * symbol in ascending order; empty slots bubble to the tail.
 */
void sort_stocks(void)
{
    int   i, j;
    Stock tmp;

    for (i = 1; i < MAX_STOCKS; i++) {
        tmp = stocks[i];
        j   = i - 1;
        while (j >= 0 && tmp.symbol[0] != '\0' &&
               (stocks[j].symbol[0] == '\0' ||
                strcmp(stocks[j].symbol, tmp.symbol) > 0)) {
            stocks[j + 1] = stocks[j];
            j--;
        }
        stocks[j + 1] = tmp;
    }
}

/**
 * @brief Refresh quote data for all populated stock slots.
 *
 * Iterates stocks[] and calls get_stock_quote() for every non-empty slot.
 */
void get_stock_quotes(void)
{
    int i;
    set_progress_message("Updating stocks");
    for (i = 0; i < MAX_STOCKS; i++) {
        if (stocks[i].symbol[0] != '\0') {
            get_stock_quote(&stocks[i]);
            update_progress_message();
        }
    }
}

/**
 * @brief Remove the stock at index and collapse the list.
 *
 * @param index Slot index of the stock to remove.
 */
void delete_stock(int index)
{
    if (index < 0 || index >= MAX_STOCKS)
        return;

    memset(&stocks[index], 0, sizeof(Stock));
    sort_stocks();
}

/**
 * @brief Split a pipe-separated symbol list into up to 5 stock slots.
 *
 * Tokens beyond 5 are ignored.
 *
 * @param buf       Pipe-separated symbol string to parse.
 * @param base_slot Starting slot index in the stocks[] array.
 */
static void parse_symbols_into_slots(const char *buf, int base_slot)
{
    char tmp[64];
    char *tok;
    int slot = base_slot;

    strncpy(tmp, buf, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    tok = strtok(tmp, "|");
    while (tok != NULL && slot < base_slot + 5) {
        strncpy(stocks[slot].symbol, tok, SYMBOL_LEN - 1);
        stocks[slot].symbol[SYMBOL_LEN - 1] = '\0';
        slot++;
        tok = strtok(NULL, "|");
    }
}

/**
 * @brief Build a pipe-separated symbol list from up to 5 stock slots.
 *
 * Empty slots are skipped.
 *
 * @param buf       Output buffer; must be at least 64 bytes.
 * @param base_slot Starting slot index in the stocks[] array.
 */
static void build_symbols_from_slots(char *buf, int base_slot)
{
    int i, pos = 0, first = 1;

    buf[0] = '\0';
    for (i = base_slot; i < base_slot + 5; i++) {
        if (stocks[i].symbol[0] == '\0')
            continue;
        if (!first)
            buf[pos++] = '|';
        strncpy(buf + pos, stocks[i].symbol, SYMBOL_LEN - 1);
        pos += strlen(stocks[i].symbol);
        first = 0;
    }
    buf[pos] = '\0';
}

/**
 * @brief Read up to 10 stock symbols from two app keys and populate stocks[].
 *
 * Key 1 fills slots 0-4; key 2 fills slots 5-9.  Each key holds a
 * pipe-separated list, e.g. "AAPL|GOOG".
 */
void load_stocks(void)
{
    char buf[64];

    if (read_appkey(buf, sizeof(buf), STOCKS_APP_KEY_1))
        parse_symbols_into_slots(buf, 0);

    if (read_appkey(buf, sizeof(buf), STOCKS_APP_KEY_2))
        parse_symbols_into_slots(buf, 5);

    sort_stocks();
}

/**
 * @brief Persist the current stocks[] symbol list to two app keys.
 *
 * Key 1 holds slots 0-4; key 2 holds slots 5-9.  Symbols are written
 * as a pipe-separated list into a 64-byte buffer.
 */
void save_stocks(void)
{
    char buf[64];

    build_symbols_from_slots(buf, 0);
    write_appkey(buf, STOCKS_APP_KEY_1);

    build_symbols_from_slots(buf, 5);
    write_appkey(buf, STOCKS_APP_KEY_2);
}
