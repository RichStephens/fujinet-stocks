#ifndef CONIO_H
#define CONIO_H

#include <fujinet-fuji.h>
#include <coco.h>

uint8_t kbhit(void);
char cgetc(void);
void cursor(bool onoff);
int wherex(void);
int wherey(void);
void gotoxy(int x, int y);
void hirestxt_init(void);
void hirestxt_close(void);
void switch_colorset(void);
void revers(uint8_t on);
#define cputs(s) putstr((s), strlen(s))
#define cputcxy(x, y, c) do { gotoxy((x), (y)); putchar(c); } while(0)
#include <hirestxt.h>
#include <chardefs.h>

#endif /* CONIO_H */
