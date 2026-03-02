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
#include <screen.h>
#include <network.h>
#include "fujinet-stocks.h"

/* -----------------------------------------------------------------------
 * Scroll buffer
 * ----------------------------------------------------------------------- */
#define SCROLL_BUF_LEN  512
static char  scroll_buf[SCROLL_BUF_LEN];
static int   scroll_len  = 0;
static int   scroll_pos  = 0;
static int   frame_count = 0;

/* -----------------------------------------------------------------------
 * fill_spaces()
 * Write n space characters.  Centralises all row-blanking loops.
 * ----------------------------------------------------------------------- */
static void fill_spaces(int n)
{
    while (n-- > 0)
        putchar(' ');
}

/* -----------------------------------------------------------------------
 * draw_ticker_row()
 * Write a SCREEN_WIDTH-character window from scroll_buf into row 0.
 * ----------------------------------------------------------------------- */
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

/* -----------------------------------------------------------------------
 * draw_menu()
 * Two-line command reference.  Written character-by-character to avoid
 * accidentally triggering a scroll via the bottom-right corner cell.
 *
 * Row 22: fill all SCREEN_WIDTH columns.
 * Row 23: stop at SCREEN_WIDTH-1 to prevent scroll on bottom-right cell.
 * ----------------------------------------------------------------------- */
static void draw_menu(void)
{
    const char *p;
    int col;

    gotoxy(0, MENU_ROW1);
    for (p = "<S>how <E>dit <D>elete <I>nfo <L>ookup", col = 0;
         *p && col < SCREEN_WIDTH; p++, col++)
        putchar(*p);
    fill_spaces(SCREEN_WIDTH - col);

    gotoxy(0, MENU_ROW2);
    for (p = "       Arrows:Select  <BREAK>:Quit       ", col = 0;
         *p && col < SCREEN_WIDTH - 1; p++, col++)
        putchar(*p);
    fill_spaces(SCREEN_WIDTH - 1 - col);
}

/* -----------------------------------------------------------------------
 * draw_slot()
 * Render one stock slot.  The "%2d: " prefix is always normal video;
 * only the 10-character symbol field is shown in inverse when highlighted.
 * An empty slot shows 10 spaces in the symbol field.
 * ----------------------------------------------------------------------- */
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

/* -----------------------------------------------------------------------
 * draw_stock_list()
 * Render all slots; called on full redraws.
 * ----------------------------------------------------------------------- */
static void draw_stock_list(int selected)
{
    int i;
    for (i = 0; i < MAX_STOCKS; i++)
        draw_slot(i, i == selected);
}

/* -----------------------------------------------------------------------
 * clear_content_area()
 * Blank rows 1-21 without touching ticker (row 0) or menu (22-23).
 * ----------------------------------------------------------------------- */
static void clear_content_area(void)
{
    int row;
    for (row = CONTENT_TOP_ROW; row <= CONTENT_BOT_ROW; row++) {
        gotoxy(0, row);
        fill_spaces(SCREEN_WIDTH);
    }
}

/* -----------------------------------------------------------------------
 * show_stock_info()
 * Uses global stock_info (filled by get_stock_info()).
 * ----------------------------------------------------------------------- */
static void show_stock_info(int selected)
{
    char price_str[12];
    char change_str[12];
    char tmp[12];
    char dir_char;
    int  col;
    const char *p;

    if (stocks[selected].symbol[0] == '\0')
        return;

    clrscr();
    get_stock_info(stocks[selected].symbol);

    format_price(stock_info.price_cents,  price_str,  sizeof(price_str));
    format_price(stock_info.change_cents, change_str, sizeof(change_str));
    switch (stock_info.direction) {
        case DIRECTION_UP:   dir_char = '^'; break;
        case DIRECTION_DOWN: dir_char = 'v'; break;
        default:             dir_char = '-'; break;
    }

    gotoxy(0, 2);  printf("Symbol  : %-10s", stock_info.symbol);
    gotoxy(0, 3);  printf("Company : %.41s", stock_info.company_name);
    gotoxy(0, 4);  printf("Exchange: %-10s", stock_info.exchange);
    gotoxy(0, 6);  printf("Price   : $%s", price_str);
    gotoxy(0, 7);  printf("Change  : %c $%s  (%u.%02u%%)",
                          dir_char, change_str,
                          stock_info.change_pct_x100 / 100,
                          stock_info.change_pct_x100 % 100);
    format_price(stock_info.prev_close_cents, tmp, sizeof(tmp));
    gotoxy(0, 9);  printf("Prev Close: $%s", tmp);
    format_price(stock_info.open_cents, tmp, sizeof(tmp));
    gotoxy(0, 10); printf("Open      : $%s", tmp);
    format_price(stock_info.high_cents, tmp, sizeof(tmp));
    gotoxy(0, 11); printf("High      : $%s", tmp);
    format_price(stock_info.low_cents, tmp, sizeof(tmp));
    gotoxy(0, 12); printf("Low       : $%s", tmp);
    gotoxy(0, 14); printf("Volume    : %ld", stock_info.volume);
    gotoxy(0, 15); printf("Updated   : %.21s", stock_info.last_updated);

    gotoxy(0, MENU_ROW1);
    for (p = "Press any key to return...", col = 0;
         *p && col < SCREEN_WIDTH; p++, col++)
        putchar(*p);
    fill_spaces(SCREEN_WIDTH - col);

    gotoxy(0, MENU_ROW2);
    fill_spaces(SCREEN_WIDTH - 1);

    while (cgetc() == 0)
        ;
}

/* -----------------------------------------------------------------------
 * edit_stock()
 * Edit a symbol in-place: position at the slot and let get_line()
 * accept input right in the symbol field.
 * ----------------------------------------------------------------------- */
static void edit_stock(int selected)
{
    char new_sym[SYMBOL_LEN];

    gotoxy(stock_coords[selected].x, stock_coords[selected].y);
    printf("%2d: ", selected + 1);

    if (get_line(new_sym, SYMBOL_LEN)) {
        strupr(new_sym);
        strncpy(stocks[selected].symbol, new_sym, SYMBOL_LEN);
        memset(&stocks[selected].price_cents, 0,
               sizeof(Stock) - SYMBOL_LEN);
    }
}

/* -----------------------------------------------------------------------
 * draw_lookup_row()
 * Renders one row from the global lookup_results.
 * ----------------------------------------------------------------------- */
static void draw_lookup_row(int i, bool highlighted)
{
    char line[41];
    snprintf(line, sizeof(line), "%-12s %-27s",
             lookup_results.results[i].displaySymbol,
             lookup_results.results[i].description);
    line[40] = '\0';
    gotoxy(0, 4 + i);
    if (highlighted) revers(1);
    printf("%s", line);
    if (highlighted) revers(0);
}

/* -----------------------------------------------------------------------
 * draw_lookup_menu()
 * ----------------------------------------------------------------------- */
static void draw_lookup_menu(void)
{
    const char *p;
    int col;

    gotoxy(0, MENU_ROW1);
    for (p = "   Arrows:Select   <ENTER>:Choose       ", col = 0;
         *p && col < SCREEN_WIDTH; p++, col++)
        putchar(*p);
    fill_spaces(SCREEN_WIDTH - col);

    gotoxy(0, MENU_ROW2);
    for (p = "              <BREAK>:Return            ", col = 0;
         *p && col < SCREEN_WIDTH - 1; p++, col++)
        putchar(*p);
    fill_spaces(SCREEN_WIDTH - 1 - col);
}

/* -----------------------------------------------------------------------
 * lookup_screen()
 * Prompt for a search string, call lookup_stock_ticker() which fills
 * global lookup_results, display results, let user choose one.
 * ----------------------------------------------------------------------- */
static void lookup_screen(int return_slot)
{
    char query[SYMBOL_LEN];
    int  lk_sel = 0;
    int  i;
    char key;

    clrscr();
    gotoxy(0, TICKER_ROW);
    printf("Lookup (enter search term): ");
    if (!get_line(query, SYMBOL_LEN) || query[0] == '\0')
        return;

    lookup_stock_ticker(query);   /* fills global lookup_results */

    clear_content_area();

    if (lookup_results.count == 0) {
        gotoxy(0, 2);
        printf("No results found.  Press any key.");
        while (cgetc() == 0)
            ;
        return;
    }

    gotoxy(0, 2);
    printf("Results for: %s", query);

    for (i = 0; i < lookup_results.count; i++)
        draw_lookup_row(i, i == lk_sel);

    draw_lookup_menu();

    for (;;) {
        while ((key = cgetc()) == 0)
            ;

        if (key == BREAK)
            break;

        if (key == ENTER) {
            memset(&stocks[return_slot], 0, sizeof(Stock));
            strncpy(stocks[return_slot].symbol,
                    lookup_results.results[lk_sel].displaySymbol,
                    SYMBOL_LEN - 1);
            break;
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

/* -----------------------------------------------------------------------
 * move_selection()
 * Grid is two columns of five rows:
 *   col 0 (left)  : slots 0, 2, 4, 6, 8
 *   col 1 (right) : slots 1, 3, 5, 7, 9
 * ----------------------------------------------------------------------- */
static int move_selection(int current, char key)
{
    switch (key) {
        case ARROW_RIGHT:
            if ((current % 2) == 0 && current + 1 < MAX_STOCKS)
                return current + 1;
            break;
        case ARROW_LEFT:
            if ((current % 2) == 1)
                return current - 1;
            break;
        case ARROW_DOWN:
            if (current + 2 < MAX_STOCKS)
                return current + 2;
            break;
        case ARROW_UP:
            if (current - 2 >= 0)
                return current - 2;
            break;
        default:
            break;
    }
    return current;
}

/* -----------------------------------------------------------------------
 * main_loop()
 * Arrow keys only redraw the two affected slots rather than the full list.
 * All other operations that change data use need_redraw.
 * ----------------------------------------------------------------------- */
void main_loop(void)
{
    bool show_stocks         = true;
    int  selected            = 0;
    int  new_sel;
    char key;
    bool need_redraw         = true;
    bool need_scroll_rebuild = true;

    clrscr();
    draw_menu();

    for (;;) {

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
            draw_menu();
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

        if (key >= 'a' && key <= 'z')
            key = (char)(key - 'a' + 'A');

        switch (key) {

            case 'S':
                show_stocks = !show_stocks;
                need_redraw = true;
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

            case 'E':
                if (show_stocks) {
                    edit_stock(selected);
                    need_scroll_rebuild = true;
                    need_redraw         = true;
                }
                break;

            case 'D':
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

            case 'I':
                if (show_stocks && stocks[selected].symbol[0] != '\0') {
                    show_stock_info(selected);
                    clrscr();
                    need_redraw         = true;
                    need_scroll_rebuild = true;
                    draw_menu();
                }
                break;

            case 'L':
                if (show_stocks) {
                    lookup_screen(selected);
                    clrscr();
                    need_redraw         = true;
                    need_scroll_rebuild = true;
                    draw_menu();
                }
                break;

            default:
                break;
        }
    }
}

/* -----------------------------------------------------------------------
 * clear_ticker_line()
 * Clear the ticker line (row 0) of the screen.
 * ----------------------------------------------------------------------- */
void clear_ticker_line(void)
{
    gotoxy(0, TICKER_ROW);
    fill_spaces(SCREEN_WIDTH - 1);
}
