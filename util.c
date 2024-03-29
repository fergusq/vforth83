#include <malloc.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>

#include "util.h"

uint8_t FINNISH = 0;

uint8_t toupperf(uint8_t chr) {
    if (FINNISH) {
        switch (chr) {
        case '{': return '[';
        case ']': return '}';
        case '|': return '\\';
        default: return toupper(chr);
        }
    } else {
        return toupper(chr);
    }
}

uint8_t *upper(uint8_t *str) {
    uint8_t *upper_str = malloc(strlen(str) + 1);
    for (int i = 0; i < strlen(str); i++) {
        upper_str[i] = toupperf(str[i]);
    }
    upper_str[strlen(str)] = 0;
    return upper_str;
}

wchar_t fix_finnish(uint16_t chr) {
    if (FINNISH) {
        switch (chr) {
        case '{': return L'ä';
        case '[': return L'Ä';
        case ']': return L'å';
        case '}': return L'Å';
        case '|': return L'ö';
        case '\\': return L'Ö';
        default: return chr;
        }
    } else {
        return chr;
    }  
}

// Data for Pearson Hash

uint8_t PEARSON_TABLE[256] = {
    238, 22, 190, 19, 101, 26, 126, 52, 130, 251, 5, 187, 57, 113, 111, 216,
    141, 123, 221, 209, 241, 189, 100, 23, 129, 224, 7, 244, 191, 80, 10, 218,
    95, 167, 170, 229, 180, 192, 109, 195, 177, 144, 178, 153, 161, 228, 98, 91,
    2, 41, 43, 33, 230, 86, 124, 92, 88, 188, 243, 219, 60, 197, 72, 186,
    171, 134, 164, 174, 32, 49, 181, 31, 211, 150, 93, 55, 127, 11, 149, 25,
    220, 160, 173, 207, 37, 21, 169, 252, 74, 50, 205, 210, 107, 135, 42, 27,
    147, 198, 137, 34, 201, 162, 154, 183, 172, 145, 120, 15, 223, 44, 118, 96,
    142, 39, 245, 16, 158, 232, 102, 254, 97, 58, 157, 131, 133, 20, 214, 236,
    176, 196, 67, 233, 14, 248, 242, 87, 222, 62, 184, 116, 212, 30, 151, 185,
    143, 234, 65, 194, 35, 90, 148, 140, 61, 51, 29, 239, 46, 12, 125, 237,
    99, 136, 114, 6, 139, 179, 70, 213, 121, 3, 82, 199, 117, 9, 168, 155,
    217, 24, 83, 112, 138, 73, 0, 36, 206, 204, 146, 59, 225, 64, 103, 249,
    110, 8, 4, 202, 40, 53, 193, 89, 105, 38, 200, 159, 250, 175, 1, 13,
    63, 203, 215, 115, 75, 76, 79, 69, 247, 81, 106, 54, 156, 78, 108, 85,
    166, 163, 227, 152, 48, 71, 165, 77, 235, 208, 66, 246, 240, 56, 17, 28,
    47, 132, 18, 68, 119, 182, 128, 45, 226, 122, 104, 253, 255, 84, 94, 231,
};

uint8_t pearson_hash(uint8_t *str) {
    size_t length = strlen(str) % 256;
    uint8_t hash = length;
    for (int i = 0; i < length; i++) {
        hash = PEARSON_TABLE[hash ^ str[i]];
    }
    return hash;
}