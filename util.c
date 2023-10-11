#include <malloc.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "util.h"

uint8_t *upper(uint8_t *str) {
    uint8_t *upper_str = malloc(strlen(str) + 1);
    for (int i = 0; i < strlen(str); i++) {
        upper_str[i] = toupper(str[i]);
    }
    upper_str[strlen(str)] = 0;
    return upper_str;
}