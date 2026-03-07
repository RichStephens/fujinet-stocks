#if defined(__CC65__)

#include <conio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "chardefs.h"
#include "get_line.h"

#ifdef __ATARI__
#include <atari.h>
#define show_cursor() do { cursor(1); OS.crsinh = 0; } while(0)
#define hide_cursor() do { cursor(0); OS.crsinh = 1; } while(0)
#else
#define show_cursor() cursor(1)
#define hide_cursor() cursor(0)
#endif

/**
 * @brief Read a line of input with echo, backspace, and cursor support.
 *
 * Handles printable characters, left-arrow/DEL as backspace, ENTER to
 * confirm, and BREAK to cancel (returns an empty string).
 *
 * @param buf     Output buffer for the entered string.
 * @param max_len Maximum number of characters (including NUL terminator).
 */
void get_line(char* buf, uint8_t max_len) {
	uint8_t c;
	uint8_t i = 0;
	uint8_t start_x = wherex();
	memset(buf, 0, max_len);

	show_cursor();
	gotox(start_x);

	do {
		c = cgetc();

		if (c == BREAK) {
			buf[0] = '\0';
			hide_cursor();
			revers(0);
			return;
		}
		else if (isprint(c)) {
			gotox(start_x + i);
			if (i == (max_len - 1)) {
				// we're at the end, set reverse and invisible cursor
				// so it looks like the cursor is editing the current last character
				revers(1);
				hide_cursor();
			} else {
				revers(0);
				show_cursor();
			}

			cputc(c);

			buf[i] = c;
			if (i < max_len - 1) {
				i++;
			}
		}
		else if ((c == CH_CURS_LEFT) || (c == CH_DEL)) {
			if (i) {
				uint8_t cur_x = wherex();

				gotox(cur_x - 1);
				if (i == (max_len - 1) && buf[i] != '\0') {
					// don't "move" the cursor, but do set back to normal text
					revers(0);
					show_cursor();
				} else {
					// we weren't on a full buffer, ok to move
					--i;
				}
				cputc(' ');
				gotox(cur_x - 1);

				buf[i] = '\0';
			}
		}

	} while (c != CH_ENTER);

	// in case last character printed left us in revers mode, undo it, and write out the final string so the inverse char is removed.
	hide_cursor();
	revers(0);
	gotox(start_x);
	cputs(buf);

	//// Alternate version just using cgets() with Oliver's changes to handle CH_DEL:
	// cgets(buf, max_len+1);
}
#endif
