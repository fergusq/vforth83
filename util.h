#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <wchar.h>

extern uint8_t FINNISH;

uint8_t toupperf(uint8_t chr);

uint8_t *upper(uint8_t *str);

wchar_t fix_finnish(uint16_t chr);

uint8_t pearson_hash(uint8_t *str);

#endif