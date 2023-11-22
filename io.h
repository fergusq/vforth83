#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdint.h>

#include "memory.h"

#define WINDOW_WIDTH 80
#define WINDOW_HEIGHT 25

#define WINDOW_WIDTH_PIXELS 640
#define WINDOW_HEIGHT_PIXELS 400

#define CELL_HEIGHT (WINDOW_HEIGHT_PIXELS / WINDOW_HEIGHT)
#define CELL_WIDTH (WINDOW_WIDTH_PIXELS / WINDOW_WIDTH)

enum InputMode {
    INPUT_MODE_ECHO,
    INPUT_MODE_NO_ECHO,
};

extern enum InputMode INPUT_MODE;

enum OutputMode {
    OUTPUT_MODE_CURSES,
    OUTPUT_MODE_SDL2,
};

extern enum OutputMode OUTPUT_MODE;

void init_io(enum OutputMode output_mode);
void free_io();

void redraw_buffer();

void forth_getyx(int *y, int *x);
void forth_printf(const char* format, ...);
void forth_addch(char character);
int forth_getch();
int forth_kbhit();
void forth_int10h(Memory* memory, uint16_t *ax, uint16_t *bx, uint16_t *cx, uint16_t *dx, uint16_t *bp, uint16_t *di);
void forth_linemove(int source, int dest);

#endif