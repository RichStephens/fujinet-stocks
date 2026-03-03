/* -----------------------------------------------------------------------
 * network.c
 * FujiNet network calls for retrieving stock data.
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fujinet-network.h>
#include "network.h"
#include "init.h"


/** @brief Global result buffers — too large for the 8-bit stack. */
LookupResults lookup_results;
StockInfo     stock_info;

/**
 * @brief Check a FujiNet return code and abort on failure.
 *
 * If err is not FN_ERR_OK, prints msg, runs platform cleanup, and exits
 * with a non-zero status.
 *
 * @param err FujiNet error code returned by a network call.
 * @param msg Human-readable description of the operation that failed.
 */
void check_error(uint8_t err, const char *msg)
{
    if (err != FN_ERR_OK) {
        printf("%s", msg);
        cleanup();
        exit(1);
    }
}

/**
 * @brief Percent-encode a string for use in a URL query parameter.
 *
 * Only unreserved characters (A-Z a-z 0-9 - _ . ~) are passed through;
 * everything else becomes %XX.
 *
 * @param src     Input string to encode.
 * @param out     Output buffer; must be at least 3*strlen(src)+1 bytes.
 * @param out_len Size of the output buffer.
 */
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


/**
 * @brief Parse a decimal string (e.g. "152.34", "-1.23") into a fixed-point
 * integer scaled by 100.
 *
 * @param s Null-terminated decimal string to parse.
 * @return Parsed value scaled by 100 (e.g. "12.34" -> 1234).
 */
static long parse_decimal(const char *s)
{
    long whole = 0, frac = 0;
    int  neg = 0, frac_digits = 0;
    const char *p = s;

    if (*p == '-') { neg = 1; p++; }
    while (*p >= '0' && *p <= '9')
        whole = whole * 10 + (*p++ - '0');
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9' && frac_digits < 2) {
            frac = frac * 10 + (*p++ - '0');
            frac_digits++;
        }
        if (frac_digits == 1) frac *= 10;
    }
    return neg ? -(whole * 100 + frac) : (whole * 100 + frac);
}

/**
 * @brief Fetch the current quote for a single stock and update its fields.
 *
 * Queries the Finnhub /quote endpoint and fills the price, change, and
 * change_pct fields of the supplied Stock record.
 *
 * @param s Pointer to the Stock record to update; symbol must be set.
 */
void get_stock_quote(Stock *s)
{
    static char url[128];
    static char val[32];

    sprintf(url, FINNHUB_STOCK_QUOTE, s->symbol, FINNHUB_API_KEY);
    check_error(network_open(url, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE),
                "Quote open failed");
    check_error(network_json_parse(url), "Quote parse failed");

    val[0] = '\0';
    network_json_query(url, "/c", val);
    s->price = parse_decimal(val);

    val[0] = '\0';
    network_json_query(url, "/d", val);
    s->change = parse_decimal(val);

    val[0] = '\0';
    network_json_query(url, "/dp", val);
    s->change_pct = (int)parse_decimal(val);

    network_close(url);
}

/**
 * @brief Fill the global stock_info struct with company profile data for symbol.
 *
 * Queries the Finnhub /stock/profile2 endpoint and populates all fields
 * of the global stock_info record.  Float fields (marketCapitalization,
 * shareOutstanding) are truncated to whole millions via atol().
 *
 * @param symbol Ticker symbol to look up.
 */
void get_stock_info(const char *symbol)
{
    static char url[128];
    static char val[64];

    memset(&stock_info, 0, sizeof(StockInfo));
    strncpy(stock_info.symbol, symbol, SYMBOL_LEN - 1);

    set_progress_message("Retrieving Info");
    sprintf(url, FINNHUB_COMPANY_PROFILE, symbol, FINNHUB_API_KEY);
    check_error(network_open(url, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE),
                "Profile open failed");
    update_progress_message();

    check_error(network_json_parse(url), "Profile parse failed");
    update_progress_message();

    val[0] = '\0';
    network_json_query(url, "/name", val);
    strncpy(stock_info.name, val, sizeof(stock_info.name) - 1);
    update_progress_message();

    val[0] = '\0';
    network_json_query(url, "/country", val);
    strncpy(stock_info.country, val, sizeof(stock_info.country) - 1);

    val[0] = '\0';
    network_json_query(url, "/currency", val);
    strncpy(stock_info.currency, val, sizeof(stock_info.currency) - 1);

    val[0] = '\0';
    network_json_query(url, "/exchange", val);
    strncpy(stock_info.exchange, val, sizeof(stock_info.exchange) - 1);

    val[0] = '\0';
    network_json_query(url, "/finnhubIndustry", val);
    strncpy(stock_info.industry, val, sizeof(stock_info.industry) - 1);

    val[0] = '\0';
    network_json_query(url, "/ipo", val);
    strncpy(stock_info.ipo, val, sizeof(stock_info.ipo) - 1);

    val[0] = '\0';
    network_json_query(url, "/phone", val);
    strncpy(stock_info.phone, val, sizeof(stock_info.phone) - 1);

    val[0] = '\0';
    network_json_query(url, "/weburl", val);
    strncpy(stock_info.weburl, val, sizeof(stock_info.weburl) - 1);

    val[0] = '\0';
    network_json_query(url, "/marketCapitalization", val);
    stock_info.market_cap = atol(val);

    val[0] = '\0';
    network_json_query(url, "/shareOutstanding", val);
    stock_info.shares_outstanding = atol(val);

    network_close(url);
}

/**
 * @brief Fill global lookup_results with up to MAX_LOOKUP_RESULTS ticker matches.
 *
 * API endpoint: https://finnhub.io/api/v1/search?q=<query>&token=<key>
 *
 * @param query    Search string (e.g. partial symbol or company name).
 */
void lookup_stock_ticker(const char *query)
{
    static char encoded[SYMBOL_LEN * 3 + 1];
    static char url[128];
    static char val[64];
    static char path[32];
    int  i, count;

    memset(&lookup_results, 0, sizeof(LookupResults));

    url_encode(query, encoded, sizeof(encoded));
    sprintf(url, FINNHUB_SYMBOL_LOOKUP, encoded, FINNHUB_API_KEY);

    check_error(network_open(url, OPEN_MODE_HTTP_GET, OPEN_TRANS_NONE), "Network open failed");
    update_progress_message();

    check_error(network_json_parse(url), "JSON parse failed");
    update_progress_message();

    val[0] = '\0';
    network_json_query(url, "/count", val);
    count = (int)atoi(val);
    if (count > MAX_LOOKUP_RESULTS)
        count = MAX_LOOKUP_RESULTS;
    lookup_results.count = count;

    for (i = 0; i < count; i++) {
        update_progress_message();
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
