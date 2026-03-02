#ifndef FUJINET_STOCKS_H
#define FUJINET_STOCKS_H

/* -----------------------------------------------------------------------
 * fujinet-stocks.h
 * Main header for the FujiNet Stocks application.
 * Multiplatform 8-bit C (cc65 / cmoc).
 * ----------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>

#include <screen.h>

/* Number of stocks the app tracks */
#define MAX_STOCKS         10

/* Maximum length of a stock ticker symbol (plus null terminator) */
#define SYMBOL_LEN         11

/* -----------------------------------------------------------------------
 * Coord  –  a 2-D screen coordinate (x = column, y = row)
 * ----------------------------------------------------------------------- */
typedef struct {
    unsigned char x;
    unsigned char y;
} Coord;

/* -----------------------------------------------------------------------
 * StockDirection  –  price movement since last update
 * ----------------------------------------------------------------------- */
typedef enum {
    DIRECTION_NONE  = 0,   /* no data / unchanged         */
    DIRECTION_UP    = 1,   /* price moved upward          */
    DIRECTION_DOWN  = 2    /* price moved downward        */
} StockDirection;

/* -----------------------------------------------------------------------
 * Stock  –  the live data record kept for each tracked symbol.
 *
 * Prices / changes are stored as integer cents to avoid floating-point
 * on 8-bit targets.  E.g. $12.34 is stored as 1234.
 * change_pct_x100 holds the percentage * 100 so 1.23% = 123.
 * ----------------------------------------------------------------------- */
typedef struct {
    char           symbol[SYMBOL_LEN];   /* ticker symbol, NUL-terminated  */
    long           price_cents;          /* current price in cents          */
    StockDirection direction;            /* up / down / none                */
    long           change_cents;         /* absolute change in cents        */
    unsigned int   change_pct_x100;      /* percent change * 100            */
} Stock;

/* -----------------------------------------------------------------------
 * StockInfo  –  richer record returned by the network info call.
 * Filled in by get_stock_info() in network.c; displayed on the info
 * screen.  The caller owns the storage.
 * ----------------------------------------------------------------------- */
typedef struct {
    char           symbol[SYMBOL_LEN];   /* ticker symbol                   */
    char           company_name[41];     /* full company name               */
    long           price_cents;          /* current price in cents          */
    long           change_cents;         /* change since prev close         */
    unsigned int   change_pct_x100;      /* percent change * 100            */
    StockDirection direction;
    long           prev_close_cents;     /* previous closing price          */
    long           open_cents;           /* today's opening price           */
    long           high_cents;           /* today's high                    */
    long           low_cents;            /* today's low                     */
    long           volume;               /* share volume                    */
    char           exchange[11];         /* exchange name                   */
    char           last_updated[21];     /* timestamp string                */
} StockInfo;

/* -----------------------------------------------------------------------
 * Global arrays (defined in fujinet-stocks.c)
 * ----------------------------------------------------------------------- */

/* The list of stocks being tracked */
extern Stock stocks[MAX_STOCKS];

/* Pre-computed screen positions for each stock slot.
 * Two columns of five rows, centred within SCREEN_WIDTH.
 *   Slots 0,2,4,6,8 → left  column (x = SLOT_LEFT_X)
 *   Slots 1,3,5,7,9 → right column (x = SLOT_RIGHT_X)
 *   SLOT_GAP (5) chars between labels
 *   Rows 5, 8, 11, 14, 17  (2 blank rows between each pair)
 */
extern const Coord stock_coords[MAX_STOCKS];


/* -----------------------------------------------------------------------
 * LookupResult / LookupResults  –  returned by lookup_stock_ticker().
 * Up to 10 matches, each carrying the displaySymbol shown on exchange
 * and the human-readable description.
 * ----------------------------------------------------------------------- */
#define MAX_LOOKUP_RESULTS  10

typedef struct {
    char description[41];    /* e.g. "APPLE INC"           */
    char displaySymbol[12];  /* e.g. "AAPL", "AAPL.SW"    */
    char symbol[12];         /* underlying symbol          */
    char type[21];           /* e.g. "Common Stock"        */
} LookupResult;

typedef struct {
    int          count;
    LookupResult results[MAX_LOOKUP_RESULTS];
} LookupResults;

/* -----------------------------------------------------------------------
 * Function prototypes
 * ----------------------------------------------------------------------- */

/* screen.c */
void main_loop(void);

/* fujinet-stocks.c */
void build_scroll_string(char *buf, int buf_len);
void format_price(long cents, char *out, int out_len);
void delete_stock(int index);

#endif /* FUJINET_STOCKS_H */