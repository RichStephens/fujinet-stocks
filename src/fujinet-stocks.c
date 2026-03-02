/* -----------------------------------------------------------------------
 * fujinet-stocks.c
 * Global data declarations and utility functions.
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <screen.h>
#include "fujinet-stocks.h"

Stock stocks[MAX_STOCKS] = {
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 },
    { "",  0L, DIRECTION_NONE, 0L, 0 }
};

/* -----------------------------------------------------------------------
 * stock_coords[]  –  pre-computed screen positions for each stock slot.
 *
 * X positions are derived from SLOT_LEFT_X / SLOT_RIGHT_X (defined in
 * screen.h) which are computed from SCREEN_WIDTH, so the layout
 * centres correctly on both 42-column (CoCo) and 40-column targets.
 *
 *   Rows 5, 8, 11, 14, 17  (2 blank rows between each pair)
 *   Vertical padding: 4 blank rows above (rows 1-4) and below (rows 18-21)
 *
 * Slot numbering goes left-to-right, top-to-bottom:
 *   0 → left/row5    1 → right/row5
 *   2 → left/row8    3 → right/row8
 *   ...
 * ----------------------------------------------------------------------- */
const Coord stock_coords[MAX_STOCKS] = {
    { SLOT_LEFT_X,   5 },   /* slot  0  (stock  1) */
    { SLOT_RIGHT_X,  5 },   /* slot  1  (stock  2) */
    { SLOT_LEFT_X,   8 },   /* slot  2  (stock  3) */
    { SLOT_RIGHT_X,  8 },   /* slot  3  (stock  4) */
    { SLOT_LEFT_X,  11 },   /* slot  4  (stock  5) */
    { SLOT_RIGHT_X, 11 },   /* slot  5  (stock  6) */
    { SLOT_LEFT_X,  14 },   /* slot  6  (stock  7) */
    { SLOT_RIGHT_X, 14 },   /* slot  7  (stock  8) */
    { SLOT_LEFT_X,  17 },   /* slot  8  (stock  9) */
    { SLOT_RIGHT_X, 17 }    /* slot  9  (stock 10) */
};

/* -----------------------------------------------------------------------
 * format_price()
 * Convert a price in integer cents to a display string, e.g.
 *   1234  →  "12.34"
 *   100   →  "1.00"
 *   50    →  "0.50"
 * out must be at least out_len bytes.
 * ----------------------------------------------------------------------- */
void format_price(long cents, char *out, int out_len)
{
    long dollars = cents / 100L;
    long rem     = cents % 100L;
    if (rem < 0) rem = -rem;
    snprintf(out, (size_t)out_len, "%ld.%02ld", dollars, rem);
}

/* -----------------------------------------------------------------------
 * build_scroll_string()
 * Build the ticker-tape string that scrolls across row 0.
 * Format per stock: "SYMBOL $PP.PP +/-CC.CC%    "
 * Only populated slots are included.
 *
 * The buffer is treated as a pure circular tape with no leading or
 * trailing blank padding.  draw_ticker_row() reads SCREEN_WIDTH chars
 * at scroll_pos using modulo wrap, so the last entry flows seamlessly
 * into the first — no blank gap, no repeated entries on screen at once.
 *
 * If total content is shorter than SCREEN_WIDTH (e.g. only one very
 * short symbol) we pad with spaces to reach exactly SCREEN_WIDTH so
 * the modulo window always sees distinct bytes and never repeats.
 * ----------------------------------------------------------------------- */
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

        format_price(stocks[i].price_cents, price_str, sizeof(price_str));

        abs_change = stocks[i].change_cents;
        if (abs_change < 0) abs_change = -abs_change;
        format_price(abs_change, change_str, sizeof(change_str));

        switch (stocks[i].direction) {
            case DIRECTION_UP:   sign_char = '+'; break;
            case DIRECTION_DOWN: sign_char = '-'; break;
            default:             sign_char = ' '; break;
        }

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

/* -----------------------------------------------------------------------
 * delete_stock()
 * Remove the stock at index from the array and collapse the list:
 * every entry after index shifts one position toward index.
 * ----------------------------------------------------------------------- */
void delete_stock(int index)
{
    int i;
    if (index < 0 || index >= MAX_STOCKS)
        return;

    for (i = index; i < MAX_STOCKS - 1; i++)
        stocks[i] = stocks[i + 1];

    memset(&stocks[MAX_STOCKS - 1], 0, sizeof(Stock));
}