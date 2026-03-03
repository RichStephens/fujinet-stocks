/* -----------------------------------------------------------------------
 * screen.c
 * Main program loop, screen rendering, and user-input handling.
 *
 * Screen layout (42 x 24):
 *   Row  0     : scrolling stock ticker tape
 *   Rows 1-21  : main content (stock list, info screen, etc.)
 *   Rows 22-23 : static command menu
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <get_line.h>
#include <screen.h>
#include <network.h>
#include "stocks.h"

/** @brief Scroll buffer and state for the ticker tape at row 0. */
#define SCROLL_BUF_LEN  512
static char  scroll_buf[SCROLL_BUF_LEN];
static int   scroll_len  = 0;
static int   scroll_pos  = 0;
static int   frame_count = 0;

/**
 * @brief Write n space characters to stdout.
 *
 * @param n Number of spaces to write.
 */
static void fill_spaces(int n)
{
    while (n-- > 0)
        putchar(' ');
}

/**
 * @brief Print a string centred within SCREEN_WIDTH on the current row.
 *
 * Leading and trailing spaces pad the field to exactly SCREEN_WIDTH columns.
 *
 * @param s The string to centre.
 */
static void print_centered(const char *s)
{
    fill_spaces((SCREEN_WIDTH - (int)strlen(s)) / 2);
    printf("%s", s);
}

/**
 * @brief Clear a single screen row.
 *
 * @param row The row number to clear.
 */
void clear_line(int row)
{
    gotoxy(0, row);
    fill_spaces(SCREEN_WIDTH);
}

/**
 * @brief Write a SCREEN_WIDTH-character window from scroll_buf into row 0.
 */
static void draw_ticker_row(void)
{
    int  col, idx;
    char c;

    gotoxy(0, TICKER_ROW);
    for (col = 0; col < SCREEN_WIDTH; col++) {
        idx = (scroll_pos + col) % scroll_len;
        c   = scroll_buf[idx];
        if ((unsigned char)c < 0x20)
            c = ' ';
        putchar(c);
    }
}

/**
 * @brief Render the two-line command reference at rows 22-23.
 *
 * Written character-by-character to avoid accidentally triggering a scroll
 * via the bottom-right corner cell.
 *
 * @param show_stocks true when the stock list is visible (ignored when lookup).
 * @param lookup      true to show the lookup screen menu instead of the main menu.
 */
static void draw_menu(bool show_stocks, bool lookup)
{
    clear_line(MENU_ROW1);
    clear_line(MENU_ROW2);

    gotoxy(0, MENU_ROW1);
    if (lookup)
        print_centered("Arrows:Select  <ENTER>:Choose");
    else if (show_stocks)
        print_centered("<H>ide <E>dit <D>elete <I>nfo <L>ookup");
    else
        print_centered("<S>how <L>ookup");

    gotoxy(0, MENU_ROW2);
    if (lookup)
        print_centered("<BREAK>:Return");
    else if (show_stocks)
        print_centered("0-9/Arrows:Select  <BREAK>:Quit");
    else
        print_centered("<BREAK>:Quit");
}

/**
 * @brief Render one stock slot at its pre-computed screen position.
 *
 * The "%2d: " prefix is always normal video; only the 10-character symbol
 * field is shown in inverse video when highlighted.  An empty slot shows
 * 10 spaces in the symbol field.
 *
 * @param i           Slot index to render.
 * @param highlighted true to render the symbol field in inverse video.
 */
static void draw_slot(int i, bool highlighted)
{
    char sym[11];

    if (stocks[i].symbol[0] != '\0') {
        snprintf(sym, sizeof(sym), "%-10s", stocks[i].symbol);
    } else {
        memset(sym, ' ', 10);
        sym[10] = '\0';
    }

    gotoxy(stock_coords[i].x, stock_coords[i].y);
    printf("%2d: ", i + 1);
    if (highlighted) revers(1);
    printf("%s", sym);
    if (highlighted) revers(0);
}

/**
 * @brief Render all stock slots; called on full redraws.
 *
 * @param selected Index of the currently selected slot.
 */
static void draw_stock_list(int selected)
{
    int i;
    for (i = 0; i < MAX_STOCKS; i++)
        draw_slot(i, i == selected);
}

/**
 * @brief Blank rows 1-21 without touching the ticker or menu rows.
 */
static void clear_content_area(void)
{
    int row;
    for (row = CONTENT_TOP_ROW; row <= CONTENT_BOT_ROW; row++) {
        gotoxy(0, row);
        fill_spaces(SCREEN_WIDTH);
    }
}

/**
 * @brief Display the detail info screen for the selected stock.
 *
 * Uses global stock_info (filled by get_stock_info()).
 *
 * @param selected Slot index of the stock to display.
 */
/**
 * @brief Format a value in millions as a human-readable string with T/B/M suffix.
 *
 * Uses integer arithmetic only (no floats).  Examples:
 *   3878463 -> "3.87T",  250000 -> "250.00B",  500 -> "500.00M"
 */
static void format_large_num(long millions, char *out, int out_len)
{
    long whole, frac;
    char suffix;

    if (millions >= 1000000L) {
        whole  = millions / 1000000L;
        frac   = (millions % 1000000L) / 10000L;
        suffix = 'T';
    } else if (millions >= 1000L) {
        whole  = millions / 1000L;
        frac   = (millions % 1000L) / 10L;
        suffix = 'B';
    } else {
        whole  = millions;
        frac   = 0;
        suffix = 'M';
    }
    snprintf(out, (size_t)out_len, "%ld.%02ld%c", whole, frac, suffix);
}

/**
 * @brief Format a raw phone digit string as a dashed phone number.
 *
 * Strips non-digit characters (e.g. trailing ".0" from the API), then:
 *   11 digits starting with 1 -> "1-XXX-XXX-XXXX"
 *   10 digits              -> "XXX-XXX-XXXX"
 *   anything else          -> raw string as-is
 */
static void format_phone(const char *raw, char *out, int out_len)
{
    char        digits[16];
    int         n = 0;
    const char *p;

    for (p = raw; *p && n < (int)sizeof(digits) - 1; p++) {
        if (*p >= '0' && *p <= '9')
            digits[n++] = *p;
    }
    digits[n] = '\0';

    if (n == 11 && digits[0] == '1')
    {
        snprintf(out, (size_t)out_len, "%c-%c%c%c-%c%c%c-%c%c%c%c",
                 digits[0],
                 digits[1], digits[2], digits[3],
                 digits[4], digits[5], digits[6],
                 digits[7], digits[8], digits[9], digits[10]);
    }
    else if (n == 10)
    {
        snprintf(out, (size_t)out_len, "%c%c%c-%c%c%c-%c%c%c%c",
                 digits[0], digits[1], digits[2],
                 digits[3], digits[4], digits[5],
                 digits[6], digits[7], digits[8], digits[9]);
    }
    else
    {
        strncpy(out, raw, (size_t)(out_len - 1));
    }
    out[out_len - 1] = '\0';
}

static void show_stock_info(int selected)
{
    char cap_str[12];
    char shares_str[12];
    char phone_str[16];

    if (stocks[selected].symbol[0] == '\0')
        return;

    clrscr();
    get_stock_info(stocks[selected].symbol);

    format_large_num(stock_info.market_cap,         cap_str,    sizeof(cap_str));
    format_large_num(stock_info.shares_outstanding, shares_str, sizeof(shares_str));
    format_phone(stock_info.phone, phone_str, sizeof(phone_str));

    gotoxy(0, 5);  printf("Symbol  : %s",  stock_info.symbol);
    gotoxy(0, 6);  printf("Company : %s",  stock_info.name);
    gotoxy(0, 7);  printf("Exchange: %s",  stock_info.exchange);
    gotoxy(0, 8);  printf("Country : %s  Currency: %s",
                          stock_info.country, stock_info.currency);
    gotoxy(0, 9);  printf("Industry: %s",  stock_info.industry);
    gotoxy(0, 10); printf("IPO     : %s",  stock_info.ipo);
    gotoxy(0, 11); printf("Mkt Cap : %s",  cap_str);
    gotoxy(0, 12); printf("Shares  : %s",  shares_str);
    gotoxy(0, 13); printf("Phone   : %s",  phone_str);
    gotoxy(0, 14); printf("Web     : %s",  stock_info.weburl);

    gotoxy(0, MENU_ROW1);
    print_centered("Press any key to return...");

    gotoxy(0, MENU_ROW2);
    fill_spaces(SCREEN_WIDTH - 1);

    while (cgetc() == 0)
        ;
}

/**
 * @brief Edit the symbol in the selected slot in-place.
 *
 * Positions at the slot and lets get_line() accept input right in the
 * symbol field.
 *
 * @param selected Slot index to edit.
 */
static void edit_stock(int selected)
{
    char new_sym[SYMBOL_LEN];

    new_sym[0] = '\0';
    gotoxy(stock_coords[selected].x, stock_coords[selected].y);
    printf("%2d: ", selected + 1);

    get_line(new_sym, SYMBOL_LEN);
    if (new_sym[0] != '\0') {
        strupr(new_sym);
        strncpy(stocks[selected].symbol, new_sym, SYMBOL_LEN);
        stocks[selected].price = 0;
        stocks[selected].change = 0;
        stocks[selected].change_pct = 0;
    }
}

/**
 * @brief Render one row from the global lookup_results.
 *
 * @param i           Result index to render.
 * @param highlighted true to render the row in inverse video.
 */
static void draw_lookup_row(int i, bool highlighted)
{
    char line[41];
    char sym[13];
    char desc[28];

    strncpy(sym,  lookup_results.results[i].displaySymbol, 12); sym[12]  = '\0';
    strncpy(desc, lookup_results.results[i].description,   27); desc[27] = '\0';
    snprintf(line, sizeof(line), "%-12s %-27s", sym, desc);
    line[40] = '\0';
    gotoxy(0, 4 + i);
    if (highlighted) revers(1);
    printf("%s", line);
    if (highlighted) revers(0);
}


/** @brief Progress message displayed on the ticker row during network calls. */
static char progress_msg[SCREEN_WIDTH + 1];

/**
 * @brief Set the progress message and display it on the ticker row.
 *
 * @param msg Message to display.
 */
void set_progress_message(const char *msg)
{
    strncpy(progress_msg, msg, sizeof(progress_msg) - 1);
    progress_msg[sizeof(progress_msg) - 1] = '\0';
    gotoxy(0, TICKER_ROW);
    printf("%s", progress_msg);
}

/**
 * @brief Append a dot to the progress message and redisplay it on the ticker row.
 */
void update_progress_message(void)
{
    if (strlen(progress_msg) < sizeof(progress_msg) - 1) {
        strcat(progress_msg, ".");
    }
    gotoxy(0, TICKER_ROW);
    printf("%s", progress_msg);
}

/**
 * @brief Prompt for a search string, display results, and let the user choose.
 *
 * Calls lookup_stock_ticker() which fills global lookup_results, then
 * displays them and allows the user to select one.
 *
 * @param return_slot   Slot index to store the chosen symbol into.
 * @param update_stocks If false, ENTER returns without modifying the stock list.
 */
static bool lookup_screen(int return_slot, bool update_stocks)
{
    char query[SYMBOL_LEN];
    int  lk_sel = 0;
    int  i;
    char key;

    query[0] = '\0';
    clrscr();
    gotoxy(0, TICKER_ROW);
    printf("Lookup (enter search term): ");
    get_line(query, SYMBOL_LEN);
    if (query[0] == '\0')
        return false;

    clear_line(TICKER_ROW);
    set_progress_message("Looking up");
    lookup_stock_ticker(query);   /* fills global lookup_results */

    clear_content_area();

    if (lookup_results.count == 0) {
        gotoxy(0, 2);
        printf("No results found.  Press any key.");
        while (cgetc() == 0)
            ;
        return false;
    }

    gotoxy(0, 2);
    printf("Results for: %s", query);

    for (i = 0; i < lookup_results.count; i++)
        draw_lookup_row(i, i == lk_sel);

    draw_menu(false, true);

    for (;;) {
        while ((key = cgetc()) == 0)
            ;

        if (key == BREAK)
            return false;

        if (key == ENTER) {
            if (!update_stocks)
                return false;
            memset(&stocks[return_slot], 0, sizeof(Stock));
            strncpy(stocks[return_slot].symbol,
                    lookup_results.results[lk_sel].displaySymbol,
                    SYMBOL_LEN - 1);
            return true;
        }

        if (key == ARROW_DOWN && lk_sel < lookup_results.count - 1) {
            draw_lookup_row(lk_sel, false);
            draw_lookup_row(++lk_sel, true);
        }

        if (key == ARROW_UP && lk_sel > 0) {
            draw_lookup_row(lk_sel, false);
            draw_lookup_row(--lk_sel, true);
        }
    }
}

/**
 * @brief Return the new selection index after a directional key press.
 *
 * Grid is two columns of five rows:
 *   col 0 (left):  slots 0-4
 *   col 1 (right): slots 5-9
 *
 * @param current Current slot index.
 * @param key     Arrow key character.
 * @return New slot index (unchanged if the move would leave the grid).
 */
static int move_selection(int current, char key)
{
    switch (key) {
        case ARROW_RIGHT:
            if (current < 5)
                return current + 5;
            break;
        case ARROW_LEFT:
            if (current >= 5)
                return current - 5;
            break;
        case ARROW_DOWN:
            if (current % 5 < 4)
                return current + 1;
            break;
        case ARROW_UP:
            if (current % 5 > 0)
                return current - 1;
            break;
        default:
            break;
    }
    return current;
}

/**
 * @brief Main application loop; returns when the user presses BREAK.
 *
 * Arrow keys only redraw the two affected slots rather than the full list.
 * All other operations that change data use need_redraw.
 */
void main_loop(void)
{
    bool    show_stocks         = true;
    int     selected            = 0;
    int     new_sel;
    char    key;
    bool    need_redraw         = true;
    bool    need_scroll_rebuild = true;
    clock_t last_quote_time     = clock();

    clrscr();
    draw_menu(show_stocks, false);

    for (;;) {

        if (clock() - last_quote_time >= GET_QUOTES_INTERVAL) {
            clear_line(TICKER_ROW);
            get_stock_quotes();
            last_quote_time     = clock();
            need_scroll_rebuild = true;
            need_redraw         = true;
        }

        if (need_scroll_rebuild) {
            build_scroll_string(scroll_buf, SCROLL_BUF_LEN);
            scroll_len          = (int)strlen(scroll_buf);
            scroll_pos          = 0;
            need_scroll_rebuild = false;
        }

        if (need_redraw) {
            clear_content_area();
            if (show_stocks)
                draw_stock_list(selected);
            draw_menu(show_stocks, false);
            need_redraw = false;
        }

        sync_frame();

        if (++frame_count >= SCROLL_FRAMES) {
            frame_count = 0;
            if (++scroll_pos >= scroll_len)
                scroll_pos = 0;
        }
        draw_ticker_row();

        key = cgetc();
        if (key == 0)
            continue;

        if (key == BREAK)
            break;

        switch (key) {

            case 'S': case 's':
                if (!show_stocks) {
                    show_stocks = true;
                    need_redraw = true;
                }
                break;

            case 'H': case 'h':
                if (show_stocks) {
                    show_stocks = false;
                    need_redraw = true;
                }
                break;

            case ARROW_UP:
            case ARROW_DOWN:
            case ARROW_LEFT:
            case ARROW_RIGHT:
                if (show_stocks) {
                    new_sel = move_selection(selected, key);
                    if (new_sel != selected) {
                        draw_slot(selected, false);
                        draw_slot(new_sel,  true);
                        selected = new_sel;
                    }
                }
                break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                if (show_stocks) {
                    new_sel = (key == '0') ? 9 : (key - '1');
                    if (new_sel != selected) {
                        draw_slot(selected, false);
                        draw_slot(new_sel,  true);
                        selected = new_sel;
                    }
                }
                break;

            case 'E': case 'e':
                if (show_stocks) {
                    edit_stock(selected);
                    sort_stocks();
                    save_stocks();
                    clear_line(TICKER_ROW);
                    get_stock_quotes();
                    last_quote_time     = clock();
                    selected            = 0;
                    need_scroll_rebuild = true;
                    need_redraw         = true;
                }
                break;

            case 'D': case 'd':
                if (show_stocks && stocks[selected].symbol[0] != '\0') {
                    delete_stock(selected);
                    if (selected >= MAX_STOCKS - 1)
                        selected = MAX_STOCKS - 2;
                    if (selected < 0)
                        selected = 0;
                    need_scroll_rebuild = true;
                    need_redraw         = true;
                }
                break;

            case 'I': case 'i':
                if (show_stocks && stocks[selected].symbol[0] != '\0') {
                    show_stock_info(selected);
                    clrscr();
                    need_redraw         = true;
                    need_scroll_rebuild = true;
                    draw_menu(show_stocks, false);
                }
                break;

            case 'L': case 'l':
                if (lookup_screen(selected, show_stocks)) {
                    clrscr();
                    sort_stocks();
                    save_stocks();
                    get_stock_quotes();
                    last_quote_time = clock();
                } else {
                    clrscr();
                }
                selected            = 0;
                need_redraw         = true;
                need_scroll_rebuild = true;
                draw_menu(show_stocks, false);
                break;

            default:
                break;
        }
    }
}
