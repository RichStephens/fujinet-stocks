/* -----------------------------------------------------------------------
 * network.c
 * FujiNet network calls for retrieving stock data.
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <screen.h>
#include <fujinet-network.h>
#include "network.h"

#define FINNHUB_API_KEY       "d6gunihr01qoor9mjbsgd6gunihr01qoor9mjbt0"
#define FINNHUB_SYMBOL_LOOKUP "N1:HTTPS://finnhub.io/api/v1/search?q=%1&token=%2"

/* Global result buffers — too large for the 8-bit stack */
LookupResults lookup_results;
StockInfo     stock_info;

/* -----------------------------------------------------------------------
 * url_encode()
 * Percent-encode a string for use in a URL query parameter.
 * Only unreserved characters (A-Z a-z 0-9 - _ . ~) are passed through;
 * everything else becomes %XX.
 * out must be at least 3*strlen(src)+1 bytes.
 * ----------------------------------------------------------------------- */
void url_encode(const char *src, char *out, int out_len)
{
    static const char hex[] = "0123456789ABCDEF";
    int pos = 0;
    unsigned char c;

    while (*src && pos < out_len - 3) {
        c = (unsigned char)*src++;
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out[pos++] = (char)c;
        } else {
            out[pos++] = '%';
            out[pos++] = hex[(c >> 4) & 0x0F];
            out[pos++] = hex[c & 0x0F];
        }
    }
    out[pos] = '\0';
}

/* -----------------------------------------------------------------------
 * build_url()
 * Fill in %1 and %2 placeholders in a URL template.
 * %1 → arg1, %2 → arg2.  Result written to out (out_len bytes).
 * ----------------------------------------------------------------------- */
static void build_url(const char *tmpl, const char *arg1, const char *arg2,
                      char *out, int out_len)
{
    const char *p = tmpl;
    int pos = 0;
    int arg_len;
    const char *arg;

    while (*p && pos < out_len - 1) {
        if (*p == '%' && (*(p+1) == '1' || *(p+1) == '2')) {
            arg = (*(p+1) == '1') ? arg1 : arg2;
            arg_len = (int)strlen(arg);
            if (pos + arg_len < out_len - 1) {
                memcpy(out + pos, arg, (size_t)arg_len);
                pos += arg_len;
            }
            p += 2;
        } else {
            out[pos++] = *p++;
        }
    }
    out[pos] = '\0';
}

/* -----------------------------------------------------------------------
 * get_stock()
 * TODO: implement with fujinet-network calls.
 * ----------------------------------------------------------------------- */
bool get_stock(int index)
{
    if (index < 0 || index >= MAX_STOCKS)
        return false;
    if (stocks[index].symbol[0] == '\0')
        return false;

    /* stub */
    stocks[index].price_cents     = 15234L;
    stocks[index].direction       = DIRECTION_UP;
    stocks[index].change_cents    = 123L;
    stocks[index].change_pct_x100 = 81;
    return true;
}

/* -----------------------------------------------------------------------
 * get_stock_info()
 * Fills the global stock_info struct for the given symbol.
 * TODO: implement with fujinet-network calls.
 * ----------------------------------------------------------------------- */
void get_stock_info(const char *symbol)
{
    memset(&stock_info, 0, sizeof(StockInfo));
    strncpy(stock_info.symbol, symbol, SYMBOL_LEN - 1);

    /* stub */
    strncpy(stock_info.company_name, "Dummy Company Inc.",  sizeof(stock_info.company_name) - 1);
    strncpy(stock_info.exchange,     "NASDAQ",              sizeof(stock_info.exchange) - 1);
    strncpy(stock_info.last_updated, "2025-01-01 12:00:00", sizeof(stock_info.last_updated) - 1);
    stock_info.price_cents      = 15234L;
    stock_info.change_cents     = 123L;
    stock_info.change_pct_x100  = 81;
    stock_info.direction        = DIRECTION_UP;
    stock_info.prev_close_cents = 15111L;
    stock_info.open_cents       = 15150L;
    stock_info.high_cents       = 15300L;
    stock_info.low_cents        = 15100L;
    stock_info.volume           = 1234567L;
}

/* -----------------------------------------------------------------------
 * lookup_stock_ticker()
 * Fills global lookup_results with up to MAX_LOOKUP_RESULTS matches.
 *
 * API endpoint: https://finnhub.io/api/v1/search?q=<query>&token=<key>
 *
 * Response JSON:
 *   { "count": N,
 *     "result": [
 *       { "description":   "APPLE INC",
 *         "displaySymbol": "AAPL", ... },
 *       ...
 *     ]
 *   }
 * ----------------------------------------------------------------------- */
void lookup_stock_ticker(const char *query)
{
    char encoded[SYMBOL_LEN * 3 + 1];
    char url[128];
    char val[64];
    char path[32];
    int  i, count;

    memset(&lookup_results, 0, sizeof(LookupResults));

    /* Show progress on ticker row */
    gotoxy(0, TICKER_ROW);
    printf("Looking up");

    url_encode(query, encoded, sizeof(encoded));
    build_url(FINNHUB_SYMBOL_LOOKUP, encoded, FINNHUB_API_KEY, url, sizeof(url));

    if (network_open(url, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE) != FN_ERR_OK)
        return;
    putchar('.');

    if (network_json_parse(url) != FN_ERR_OK) {
        network_close(url);
        return;
    }
    putchar('.');

    val[0] = '\0';
    network_json_query(url, "/count", val);
    count = (int)atoi(val);
    if (count > MAX_LOOKUP_RESULTS)
        count = MAX_LOOKUP_RESULTS;
    lookup_results.count = count;

    for (i = 0; i < count; i++) {
        putchar('.');
        snprintf(path, sizeof(path), "/result/%d/description", i);
        val[0] = '\0';
        network_json_query(url, path, val);
        strncpy(lookup_results.results[i].description, val,
                sizeof(lookup_results.results[i].description) - 1);

        snprintf(path, sizeof(path), "/result/%d/displaySymbol", i);
        val[0] = '\0';
        network_json_query(url, path, val);
        strncpy(lookup_results.results[i].displaySymbol, val,
                sizeof(lookup_results.results[i].displaySymbol) - 1);
    }

    network_close(url);
}
