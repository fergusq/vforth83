#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>

#include "memory.h"

#define WINDOW_WIDTH 80
#define WINDOW_HEIGHT 25

enum InputMode {
    INPUT_MODE_ECHO,
    INPUT_MODE_NO_ECHO,
};

extern enum InputMode INPUT_MODE;

void init_io();
void free_io();

void forth_getyx(int *y, int *x);
void forth_printf(const char* format, ...);
void forth_addch(char character);
int forth_getch();
int forth_kbhit();
void forth_int10h(Memory* memory, uint16_t *ax, uint16_t *bx, uint16_t *cx, uint16_t *dx, uint16_t *bp, uint16_t *di);

#endif