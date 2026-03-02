#ifndef NETWORK_H
#define NETWORK_H

/* -----------------------------------------------------------------------
 * network.h
 * FujiNet network calls for retrieving stock data.
 * ----------------------------------------------------------------------- */

#include <stdbool.h>
#include "fujinet-stocks.h"

/* Global result buffers — defined in network.c, too large for the stack */
extern LookupResults lookup_results;
extern StockInfo     stock_info;

/* URL-encode src into out (out must be >= 3*strlen(src)+1 bytes). */
void url_encode(const char *src, char *out, int out_len);

/* Fetch current price/change data for stocks[index] and update in-place. */
bool get_stock(int index);

/* Fill global stock_info with detailed quote data for symbol. */
void get_stock_info(const char *symbol);

/* Fill global lookup_results with up to MAX_LOOKUP_RESULTS ticker matches. */
void lookup_stock_ticker(const char *query);

#endif /* NETWORK_H */
