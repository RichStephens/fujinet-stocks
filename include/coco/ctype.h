#ifndef CTYPE_H
#define CTYPE_H
#include <stdint.h>

#define isspace(c) ((c)==' ' || ((uint8_t)(c)>=9 && (uint8_t)(c)<=13))
#define isprint(c) (c>=0x20 && c<=0x8E)

#endif /* CTYPE_H */
