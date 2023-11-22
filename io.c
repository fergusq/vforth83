#define _XOPEN_SOURCE_EXTENDED

#ifndef __EMSCRIPTEN__
    #define ENABLE_CURSES
    #include <termio.h>
#else
    #include <emscripten.h>
#endif

#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <wchar.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iconv.h>

#ifdef ENABLE_CURSES
//#include <ncursesw/curses.h>
#include <ncurses.h>
#endif

#include "forth.h"
#include "io.h"
#include "memory.h"

#define modulo(i, n) (((i) % (n) + n) % (n))

wchar_t *CP850_TO_UNICODE[] = {
    [0x80] = L"Ç",
    [0x81] = L"ü",
    [0x82] = L"é",
    [0x83] = L"â",
    [0x84] = L"ä",
    [0x85] = L"à",
    [0x86] = L"å",
    [0x87] = L"ç",
    [0x88] = L"ê",
    [0x89] = L"ë",
    [0x8A] = L"è",
    [0x8B] = L"ï",
    [0x8C] = L"î",
    [0x8D] = L"ì",
    [0x8E] = L"Ä",
    [0x8F] = L"Å",
    [0x90] = L"É",
    [0x91] = L"æ",
    [0x92] = L"Æ",
    [0x93] = L"ô",
    [0x94] = L"ö",
    [0x95] = L"ò",
    [0x96] = L"û",
    [0x97] = L"ù",
    [0x98] = L"ÿ",
    [0x99] = L"Ö",
    [0x9A] = L"Ü",
    [0x9B] = L"ø",
    [0x9C] = L"£",
    [0x9D] = L"Ø",
    [0x9E] = L"×",
    [0x9F] = L"ƒ",
    [0xA0] = L"á",
    [0xA1] = L"í",
    [0xA2] = L"ó",
    [0xA3] = L"ú",
    [0xA4] = L"ñ",
    [0xA5] = L"Ñ",
    [0xA6] = L"ª",
    [0xA7] = L"º",
    [0xA8] = L"¿",
    [0xA9] = L"®",
    [0xAA] = L"¬",
    [0xAB] = L"½",
    [0xAC] = L"¼",
    [0xAD] = L"¡",
    [0xAE] = L"«",
    [0xAF] = L"»",
    [0xB0] = L"░",
    [0xB1] = L"▒",
    [0xB2] = L"▓",
    [0xB3] = L"│",
    [0xB4] = L"┤",
    [0xB5] = L"Á",
    [0xB6] = L"Â",
    [0xB7] = L"À",
    [0xB8] = L"©",
    [0xB9] = L"╣",
    [0xBA] = L"║",
    [0xBB] = L"╗",
    [0xBC] = L"╝",
    [0xBD] = L"¢",
    [0xBE] = L"¥",
    [0xBF] = L"┐",
    [0xC0] = L"└",
    [0xC1] = L"┴",
    [0xC2] = L"┬",
    [0xC3] = L"├",
    [0xC4] = L"─",
    [0xC5] = L"┼",
    [0xC6] = L"ã",
    [0xC7] = L"Ã",
    [0xC8] = L"╚",
    [0xC9] = L"╔",
    [0xCA] = L"╩",
    [0xCB] = L"╦",
    [0xCC] = L"╠",
    [0xCD] = L"═",
    [0xCE] = L"╬",
    [0xCF] = L"¤",
    [0xD0] = L"ð",
    [0xD1] = L"Ð",
    [0xD2] = L"Ê",
    [0xD3] = L"Ë",
    [0xD4] = L"È",
    [0xD5] = L"ı",
    [0xD6] = L"Í",
    [0xD7] = L"Î",
    [0xD8] = L"Ï",
    [0xD9] = L"┘",
    [0xDA] = L"┌",
    [0xDB] = L"█",
    [0xDC] = L"▄",
    [0xDD] = L"¦",
    [0xDE] = L"Ì",
    [0xDF] = L"▀",
    [0xE0] = L"Ó",
    [0xE1] = L"ß",
    [0xE2] = L"Ô",
    [0xE3] = L"Ò",
    [0xE4] = L"õ",
    [0xE5] = L"Õ",
    [0xE6] = L"µ",
    [0xE7] = L"þ",
    [0xE8] = L"Þ",
    [0xE9] = L"Ú",
    [0xEA] = L"Û",
    [0xEB] = L"Ù",
    [0xEC] = L"ý",
    [0xED] = L"Ý",
    [0xEE] = L"¯",
    [0xEF] = L"´",
    [0xF0] = L"\xAD",
    [0xF1] = L"±",
    [0xF2] = L"‗",
    [0xF3] = L"¾",
    [0xF4] = L"¶",
    [0xF5] = L"§",
    [0xF6] = L"÷",
    [0xF7] = L"¸",
    [0xF8] = L"°",
    [0xF9] = L"¨",
    [0xFA] = L"·",
    [0xFB] = L"¹",
    [0xFC] = L"³",
    [0xFD] = L"²",
    [0xFE] = L"■",
    [0xFF] = L"\xA0",
};

char *CP850_TO_UTF16[256];

// For the Finnish keyboard
uint8_t SHIFTED_CHARACTERS[256] = {
    ['1'] = '!',
    ['2'] = '\"',
    ['3'] = '#',
    ['4'] = '$',
    ['5'] = '%',
    ['6'] = '&',
    ['7'] = '/',
    ['8'] = '(',
    ['9'] = ')',
    ['0'] = '=',
    ['+'] = '?',
    ['<'] = '>',
    ['\''] = '*',
    [','] = ';',
    ['.'] = ':',
    ['-'] = '_',
};

// For the Finnish keyboard
uint8_t ALT_GR_CHARACTERS[256] = {
    ['2'] = '@',
    ['4'] = '$',
    ['7'] = '{',
    ['8'] = '[',
    ['9'] = ']',
    ['0'] = '}',
    ['+'] = '\\',
};

uint8_t IS_INITIALIZED = 0;
enum InputMode INPUT_MODE = 0;
enum OutputMode OUTPUT_MODE = 0;

#ifdef ENABLE_CURSES
// Curses-specific data
WINDOW* OUTPUT_WINDOW;
#endif

// SDL2-specific data
SDL_Window* SDL_WINDOW;
SDL_Renderer* SDL_RENDERER;
SDL_Texture* SDL_TEXTURE_GRAPHICS;
TTF_Font *SDL_FONT;

// Buffers
uint8_t SCREEN_BUFFER[WINDOW_HEIGHT + 1][WINDOW_WIDTH + 1];
uint32_t GRAPHICS_SCREEN_BUFFER[2][WINDOW_HEIGHT_PIXELS * WINDOW_WIDTH_PIXELS];
uint8_t DRAWING_PAGE = 0, DISPLAY_PAGE = 0, DRAWING_FUNCTION = 2;

uint8_t CURSOR_Y = 0, CURSOR_X = 0;
uint8_t PEN_Y = 0, PEN_X = 0;

void init_io(enum OutputMode output_mode) {
    if (IS_INITIALIZED) return;
    IS_INITIALIZED = 1;
    INPUT_MODE = INPUT_MODE_NO_ECHO;
    OUTPUT_MODE = output_mode;

    if (output_mode == OUTPUT_MODE_CURSES) {
        #ifdef ENABLE_CURSES
        initscr();
        cbreak();
        noecho();
        nonl();

        int y = (LINES-WINDOW_HEIGHT)/2, x = (COLS-WINDOW_WIDTH)/2;

        // Draw border around output window
        mvhline(y-1,             x-1,            ACS_HLINE, WINDOW_WIDTH+2);
        mvhline(y+WINDOW_HEIGHT, x-1,            ACS_HLINE, WINDOW_WIDTH+2);
        mvvline(y-1,             x-1,            ACS_VLINE, WINDOW_HEIGHT+2);
        mvvline(y-1,             x+WINDOW_WIDTH, ACS_VLINE, WINDOW_HEIGHT+2);
        mvaddch(y-1,             x-1,            ACS_ULCORNER);
        mvaddch(y-1,             x+WINDOW_WIDTH, ACS_URCORNER);
        mvaddch(y+WINDOW_HEIGHT, x-1,            ACS_LLCORNER);
        mvaddch(y+WINDOW_HEIGHT, x+WINDOW_WIDTH, ACS_LRCORNER);
        
        refresh();

        OUTPUT_WINDOW = newwin(WINDOW_HEIGHT, WINDOW_WIDTH, y, x);
        scrollok(OUTPUT_WINDOW, TRUE);
        keypad(OUTPUT_WINDOW, TRUE);
        //nodelay(OUTPUT_WINDOW, TRUE);

        // Disable IXON and IXOFF so that C-q and C-s can be used by the program
        struct termios term;
        tcgetattr(STDIN_FILENO, &term);
        term.c_iflag &= ~(IXON | IXOFF);
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
        #endif
    } else if (output_mode == OUTPUT_MODE_SDL2) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        SDL_WINDOW = SDL_CreateWindow("Forth", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        SDL_RENDERER = SDL_CreateRenderer(SDL_WINDOW, -1, SDL_RENDERER_ACCELERATED);
        SDL_TEXTURE_GRAPHICS = SDL_CreateTexture(SDL_RENDERER, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
        SDL_RenderSetLogicalSize(SDL_RENDERER, WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);

        TTF_Init();
        SDL_FONT = TTF_OpenFont("MxPlus_IBM_VGA_8x14.ttf", 16);
    }

    // Convert CP850 characters to UTF-16
    iconv_t cd = iconv_open("UTF-16", "CP850");
    for (int i = 0; i < 256; i++) {
        if (CP850_TO_UNICODE[i] != NULL) {
            size_t inbytesleft = 1, outbytesleft = 4;
            char *inbuf = (char*)&i;
            char *outbuf = CP850_TO_UTF16[i] = malloc(4);
            iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        }
    }
    iconv_close(cd);
}

void free_io() {
    if (OUTPUT_MODE == OUTPUT_MODE_CURSES) {
        #ifdef ENABLE_CURSES
        delwin(OUTPUT_WINDOW);
        endwin();
        #endif
    } else if (OUTPUT_MODE == OUTPUT_MODE_SDL2) {
        SDL_DestroyTexture(SDL_TEXTURE_GRAPHICS);
        SDL_DestroyRenderer(SDL_RENDERER);
        SDL_DestroyWindow(SDL_WINDOW);
        SDL_Quit();
        TTF_Quit();
    }
}

// Buffer commands

void addch_buffer(uint16_t character) {
    character &= 0xFF; // TODO: attributes
    if (character == 8) { // Backspace
        SCREEN_BUFFER[CURSOR_Y][CURSOR_X] = 0;
        if (CURSOR_X > 0) {
            CURSOR_X -= 1;
        } else if (CURSOR_Y > 0) {
            CURSOR_Y -= 1;
            CURSOR_X = WINDOW_WIDTH - 1;
        }
    } else {
        SCREEN_BUFFER[CURSOR_Y][CURSOR_X] = character;
        if (CURSOR_X < WINDOW_WIDTH - 1) {
            CURSOR_X += 1;
        } else if (CURSOR_Y < WINDOW_HEIGHT - 1) {
            CURSOR_Y += 1;
            CURSOR_X = 0;
        }
    }
}

uint8_t get_pixel_buffer(int page, int y, int x) {
    y = modulo(y, WINDOW_HEIGHT_PIXELS);
    x = modulo(x, WINDOW_WIDTH_PIXELS);
    return GRAPHICS_SCREEN_BUFFER[page][2*y*WINDOW_WIDTH_PIXELS + x] & 0xF ? 0xF : 0 ;
}

void set_pixel_buffer(int page, int y, int x, uint8_t value) {
    y = modulo(y, WINDOW_HEIGHT_PIXELS);
    x = modulo(x, WINDOW_WIDTH_PIXELS);
    GRAPHICS_SCREEN_BUFFER[page][2*y*WINDOW_WIDTH_PIXELS + x] = value&0xF ? 0xFFFFFFFF : 0xFF000000;
    GRAPHICS_SCREEN_BUFFER[page][(2*y+1)*WINDOW_WIDTH_PIXELS + x] = value&0xF ? 0xFFFFFFFF : 0xFF000000;

    if (OUTPUT_MODE == OUTPUT_MODE_CURSES) {
        // In the curses mode, we will update the screen buffer to render the pixels with block characters
        int cell_x = x / CELL_WIDTH;
        int cell_y = y*2 / CELL_HEIGHT;

        // Calculate upper half of cell
        uint8_t upper_half = 0;
        for (int i = 0; i < CELL_HEIGHT/2; i++) {
            for (int j = 0; j < CELL_WIDTH; j++) {
                upper_half |= GRAPHICS_SCREEN_BUFFER[DISPLAY_PAGE][(cell_y*CELL_HEIGHT+i)*WINDOW_WIDTH_PIXELS + cell_x*CELL_WIDTH+j] & 1;
            }
        }

        // Calculate lower half of cell
        uint8_t lower_half = 0;
        for (int i = CELL_HEIGHT/2; i < CELL_HEIGHT; i++) {
            for (int j = 0; j < CELL_WIDTH; j++) {
                lower_half |= GRAPHICS_SCREEN_BUFFER[DISPLAY_PAGE][(cell_y*CELL_HEIGHT+i)*WINDOW_WIDTH_PIXELS + cell_x*CELL_WIDTH+j] & 1;
            }
        }

        // Draw cell
        if (upper_half == 0 && lower_half == 0) {
            SCREEN_BUFFER[cell_y][cell_x] = 0;
        } else if (upper_half == 1 && lower_half == 0) {
            SCREEN_BUFFER[cell_y][cell_x] = 0xDF;
        } else if (upper_half == 0 && lower_half == 1) {
            SCREEN_BUFFER[cell_y][cell_x] = 0xDC;
        } else if (upper_half == 1 && lower_half == 1) {
            SCREEN_BUFFER[cell_y][cell_x] = 0xDB;
        }

        #ifdef ENABLE_CURSES
        mvprintw(0, 0, "x = %d, y = %d, cell_x = %d, cell_y = %d, upper_half = %d, lower_half = %d, character = %d", x, y, cell_x, cell_y, upper_half, lower_half, SCREEN_BUFFER[cell_y][cell_x]);
        refresh();
        #endif
    }
}

void draw_pixel_buffer(int y, int x) {
    y = modulo(y, WINDOW_HEIGHT_PIXELS);
    x = modulo(x, WINDOW_WIDTH_PIXELS);
    if (DRAWING_FUNCTION == 0) {
        set_pixel_buffer(DRAWING_PAGE, y, x, 0);
    } else if (DRAWING_FUNCTION == 1) {
        set_pixel_buffer(DRAWING_PAGE, y, x, 1);
    } else if (DRAWING_FUNCTION == 2) {
        set_pixel_buffer(DRAWING_PAGE, y, x, GRAPHICS_SCREEN_BUFFER[DRAWING_PAGE][y*WINDOW_WIDTH_PIXELS + x]&1 ? 0 : 1);
    }
}

void empty_buffer() {
    // Empty SCREEN_BUFFER
    CURSOR_Y = 0;
    CURSOR_X = 0;
    for (int i = 0; i < WINDOW_HEIGHT; i++) {
        for (int j = 0; j < WINDOW_WIDTH; j++) {
            SCREEN_BUFFER[i][j] = 0;
        }
    }

    // Empty GRAPHICS_SCREEN_BUFFER
    PEN_Y = 0;
    PEN_X = 0;
    for (int i = 0; i < WINDOW_HEIGHT_PIXELS; i++) {
        for (int j = 0; j < WINDOW_WIDTH_PIXELS; j++) {
            GRAPHICS_SCREEN_BUFFER[DRAWING_PAGE][i*WINDOW_WIDTH_PIXELS + j] = 0;
        }
    }
}

void redraw_buffer() {
    if (OUTPUT_MODE == OUTPUT_MODE_CURSES) {
        #ifdef ENABLE_CURSES
        werase(OUTPUT_WINDOW);

        // Draw SCREEN_BUFFER
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                uint8_t character = SCREEN_BUFFER[y][x];
                if (character >= 0x80) {
                    cchar_t ch = { 0 };
                    setcchar(&ch, CP850_TO_UNICODE[character], WA_NORMAL, 0, NULL);
                    mvwadd_wch(OUTPUT_WINDOW, y, x, &ch); // TODO attributes
                } else if (character != 0 && character != '\n' && character != '\r') {
                    mvwinsch(OUTPUT_WINDOW, y, x, character);
                } else {
                    mvwinsch(OUTPUT_WINDOW, y, x, ' ');
                }
            }
        }
        wmove(OUTPUT_WINDOW, CURSOR_Y, CURSOR_X);
        wrefresh(OUTPUT_WINDOW);
        #endif
    } else if (OUTPUT_MODE == OUTPUT_MODE_SDL2) {
        // Clear screen
        SDL_SetRenderDrawColor(SDL_RENDERER, 0, 0, 0, 255);
        SDL_RenderClear(SDL_RENDERER);

        // Draw GRAPHICS_SCREEN_BUFFER
        SDL_UpdateTexture(SDL_TEXTURE_GRAPHICS, NULL, GRAPHICS_SCREEN_BUFFER, WINDOW_WIDTH_PIXELS * sizeof(uint32_t));
        SDL_SetRenderDrawColor(SDL_RENDERER, 0, 0, 0, 255);
        SDL_RenderClear(SDL_RENDERER);
        SDL_RenderCopy(SDL_RENDERER, SDL_TEXTURE_GRAPHICS, NULL, NULL);

        // Draw SCREEN_BUFFER
        SDL_Color color = { 255, 255, 255, 255 };
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                uint8_t character = SCREEN_BUFFER[y][x];
                if (character >= 0x80) {
                    char *utf16 = CP850_TO_UTF16[character];
                    SDL_Surface *surface = TTF_RenderUNICODE_Solid(SDL_FONT, (uint16_t*)utf16, color);
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(SDL_RENDERER, surface);
                    SDL_Rect rect = { x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT };
                    SDL_RenderCopy(SDL_RENDERER, texture, NULL, &rect);
                    SDL_DestroyTexture(texture);
                    SDL_FreeSurface(surface);
                } else if (character != 0 && character != '\n' && character != '\r') {
                    char text[2] = { character, 0 };
                    SDL_Surface *surface = TTF_RenderText_Solid(SDL_FONT, text, color);
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(SDL_RENDERER, surface);
                    SDL_Rect rect = { x*CELL_WIDTH, y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT };
                    SDL_RenderCopy(SDL_RENDERER, texture, NULL, &rect);
                    SDL_DestroyTexture(texture);
                    SDL_FreeSurface(surface);
                }
            }
        }

        // Draw cursor
        SDL_Rect rect = { CURSOR_X*CELL_WIDTH, CURSOR_Y*CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT };
        SDL_SetRenderDrawColor(SDL_RENDERER, 255, 255, 255, 255);
        SDL_RenderDrawRect(SDL_RENDERER, &rect);

        SDL_RenderPresent(SDL_RENDERER);
    }
}

// Interface for rest of the Forth interpreter

void forth_getyx(int *y, int *x) {
    *y = CURSOR_Y;
    *x = CURSOR_X;
}

char OUTPUT_BUFFER[80];

void forth_printf(const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int size = vsnprintf(OUTPUT_BUFFER, 80, format, argptr);
    if (size >= 0) {
        if (IS_INITIALIZED) {
            for (int i = 0; i < size; i++) {
                addch_buffer(OUTPUT_BUFFER[i]);
            }
            redraw_buffer();
        } else {
            printf("%s", OUTPUT_BUFFER);
        }
    }
    va_end(argptr);
}

void forth_addch(char character) {
    if (IS_INITIALIZED) {
        addch_buffer(character);
        redraw_buffer();
    } else {
        putchar(character);
    }
}

int buffer;
SDL_Keysym keysym;

int forth_getch() {
    if (IS_INITIALIZED) {
        if (buffer != 0) {
            int result = buffer;
            buffer = 0;
            return result;
        }
        #ifdef ENABLE_CURSES
        if (OUTPUT_MODE == OUTPUT_MODE_CURSES) {
            int key = wgetch(OUTPUT_WINDOW);
            if (key == KEY_UP) {
                buffer = 72;
                return 0;
            } else if (key == KEY_DOWN) {
                buffer = 80;
                return 0;
            } else if (key == KEY_LEFT) {
                buffer = 75;
                return 0;
            } else if (key == KEY_RIGHT) {
                buffer = 77;
                return 0;
            } else if (key == KEY_BACKSPACE) {
                return 8;
            } else if (key == KEY_DC) {
                buffer = 71;
                return 0;
            } else if (key == KEY_CLEAR) {
                buffer = 117;
                return 0;
            } else if (key == KEY_IL) {
                buffer = 118;
                return 0;
            } else if (key == KEY_DL) {
                buffer = 119;
                return 0;
            }
            return key;
        } else
        #endif
        if (OUTPUT_MODE == OUTPUT_MODE_SDL2) {
            int ans;
            if (keysym.sym == SDLK_UP) {
                buffer = 72;
                ans = 0;
            } else if (keysym.sym == SDLK_DOWN) {
                buffer = 80;
                ans = 0;
            } else if (keysym.sym == SDLK_LEFT) {
                buffer = 75;
                ans = 0;
            } else if (keysym.sym == SDLK_RIGHT) {
                buffer = 77;
                ans = 0;
            } else if (keysym.sym == SDLK_BACKSPACE) {
                ans = 8;
            } else if (keysym.sym == SDLK_RETURN) {
                ans = '\r';
            } else {
                // TODO other special symbols
                ans = keysym.sym;
                if (keysym.mod & KMOD_SHIFT) {
                    if (SHIFTED_CHARACTERS[ans] != 0) {
                        ans = SHIFTED_CHARACTERS[ans];
                    }
                } else if (keysym.mod & KMOD_RALT) {
                    if (ALT_GR_CHARACTERS[ans] != 0) {
                        ans = ALT_GR_CHARACTERS[ans];
                    }
                }
            }
            keysym.sym = 0;
            return ans;
        }
    } else {
        return getchar();
    }
}

int forth_kbhit() {
    if (buffer != 0) {
        return 1;
    }

    if (OUTPUT_MODE == OUTPUT_MODE_CURSES || !IS_INITIALIZED) {
        struct timeval tv;
        fd_set rdfs;

        tv.tv_sec = 0;
        tv.tv_usec = 0;

        FD_ZERO(&rdfs);
        FD_SET(STDIN_FILENO, &rdfs);

        select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
        return FD_ISSET(STDIN_FILENO, &rdfs);
    } else if (OUTPUT_MODE == OUTPUT_MODE_SDL2) {
        #ifdef __EMSCRIPTEN__
        emscripten_sleep(10);
        #endif
        if (keysym.sym != 0) return 1;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_LCTRL
                    || event.key.keysym.sym == SDLK_RCTRL
                    || event.key.keysym.sym == SDLK_LSHIFT
                    || event.key.keysym.sym == SDLK_RSHIFT
                    || event.key.keysym.sym == SDLK_LALT
                    || event.key.keysym.sym == SDLK_RALT) {
                    continue;
                }
                //printf("Key pressed: %d\n", event.key.keysym.sym);
                keysym = event.key.keysym;
            } else if (event.type == SDL_QUIT) {
                free_io();
                exit(0);
            }
        }
        redraw_buffer();
        return 0;
    }
}

void copyline(int sourcerow, int sourcecol, int sourcelen, int destrow, int destcol) {
    for (int i = 0; i < sourcelen; i++) {
        SCREEN_BUFFER[destrow][destcol+i] = SCREEN_BUFFER[sourcerow][sourcecol+i];
    }
}

void forth_int10h(Memory* memory, uint16_t *ax, uint16_t *bx, uint16_t *cx, uint16_t *dx, uint16_t *bp, uint16_t *di) {
    uint16_t ah = *ax >> 8, al = *ax & 0xFF;
    uint16_t bh = *bx >> 8, bl = *bx & 0xFF;
    uint16_t ch = *cx >> 8, cl = *cx & 0xFF;
    uint16_t dh = *dx >> 8, dl = *dx & 0xFF;
    if (!IS_INITIALIZED) {
        // Fallback if we are in non-curses mode
        switch (ah) {
        case 0x09:
        case 0x0A:
            putchar(al);
            break;
        case 0x13:
            for (uint16_t i = 0; i < *cx; i++) {
                uint8_t character = *memory_at8(memory, *bp + i);
                putchar(character);
            }
            break;
        default:
            break;
        }
        return;
    }
    // Trace the call for debug purposes
    if (1) {
        FILE *logfile = fopen("trace.log", "a");
        fprintf(logfile, "INT 10h: AH = 0x%02X AL = 0x%02X BX = 0x%04X CX = 0x%04X DX = 0x%04X BP = 0x%04X DI = 0x%04X\n", ah, al, *bx, *cx, *dx, *bp, *di);
        print_stack_trace(logfile);
        fprintf(logfile, "\n\n");
        fclose(logfile);
    }
    int y, x;
    //wprintw(OUTPUT_WINDOW, "INT 10h: AH = 0x%02X\n", ah);
    switch (ah) {
    case 0x00:
        // Set video mode
        // Not implemented since graphics not possible with NCurses
        empty_buffer();
        break;
    case 0x01:
        // Set cursor type
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x02:
        // Set cursor position
        // DH = Row
        // DL = Column
        CURSOR_Y = dh;
        CURSOR_X = dl;
        break;
    case 0x03:
        // Get cursor position and size
        *dx = ((CURSOR_Y & 0xFF) << 8) | (CURSOR_X & 0xFF);
        break;
    case 0x04:
        // Read light pen position
        // Not implemented since light pens are not supported by NCurses
        break;
    case 0x05:
        // Select active display page
        // Not implemented
        break;
    case 0x06:
        // Scroll up window
        // AL = number of lines by which to scroll up (00h = clear entire window)
        // BH = attribute used to write blank lines at bottom of window
        // CH,CL = row,column of window's upper left corner
        // DH,DL = row,column of window's lower right corner
        y = CURSOR_Y;
        x = CURSOR_X;
        if (al == 0) {
            empty_buffer();
        } else {
            for (int i = al; i < dh-ch+1; i++) {
                copyline(ch+i, cl, dl-cl+1, ch+i-al, cl);
            }
            for (int i = 0; i < al; i++) {
                for (int j = 0; j < dl-cl+1; j++) {
                    SCREEN_BUFFER[dh-i][cl+j] = (bh << 8);
                }
            }
        }
        CURSOR_Y = y;
        CURSOR_X = x;
        break;
    case 0x07:
        // Scroll down window
        // AL = number of lines by which to scroll down (00h = clear entire window)
        // BH = attribute used to write blank lines at top of window
        // CH,CL = row,column of window's upper left corner
        // DH,DL = row,column of window's lower right corner
        y = CURSOR_Y;
        x = CURSOR_X;
        if (al == 0) {
            empty_buffer();
        } else {
            for (int i = al; i < dh-ch+1; i++) {
                copyline(dh-i, cl, dl-cl+1, dh-i+al, cl);
            }
            for (int i = 0; i < al; i++) {
                for (int j = 0; j < dl-cl+1; j++) {
                    SCREEN_BUFFER[ch+i][cl+j] = (bh << 8);
                }
            }
        }
        CURSOR_Y = y;
        CURSOR_X = x;
        break;
    case 0x08:
        // Read character and attribute at cursor
        {
            uint8_t ch = SCREEN_BUFFER[CURSOR_Y][CURSOR_X];
            *ax = ((ch & 0xFF00) << 8) | (ch & 0xFF);
        }
        break;
    case 0x09:
        // Write character and attribute at cursor
        // AL = Character
        // BH = Page
        // BL = Attribute
        // CX = Number of times to write
        y = CURSOR_Y;
        x = CURSOR_X;
        for (uint16_t i = 0; i < *cx; i++) {
            addch_buffer(al | (bl << 8));
        }
        CURSOR_Y = y;
        CURSOR_X = x;
        break;
    case 0x0A:
        // Write character at cursor
        // AL = Character
        // BH = Page
        // CX = Number of times to write
        y = CURSOR_Y;
        x = CURSOR_X;
        for (uint16_t i = 0; i < *cx; i++) {
            addch_buffer(al);
        }
        CURSOR_Y = y;
        CURSOR_X = x;
        break;
    case 0x0B:
        // Set color palette
        // Not implemented
        break;
    case 0x0C:
        // Write graphics pixel
        // BH = page number
        // AL = pixel color
        // (bit 7 => xor)
        // CX = column
        // DX = row
        {
            if (al & 0b10000000) {
                al &= 0b01111111;
                uint8_t pixel = get_pixel_buffer(bh, *dx, *cx);
                al ^= pixel;
            }
            set_pixel_buffer(bh, *dx, *cx, al);
        }
        break;
    case 0x0D:
        // Read graphics pixel
        // BH = page number
        // CX = column
        // DX = row
        {
            uint8_t pixel = get_pixel_buffer(bh, *dx, *cx);
            *ax |= pixel << 8;
        }
        break;
    case 0x0E:
        // Write character in teletype mode
        // Not implemented
        break;
    case 0x0F:
        // Get video mode
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x11:
        // Change text mode character set
        // Not implemented
        break;
    case 0x13:
        // Write string
        // AL = Attribute
        // BH = Page
        // CX = Number of characters in string
        // DH = Row
        // DL = Column
        // ES:BP = String
        y = CURSOR_Y;
        x = CURSOR_X;
        for (uint16_t i = 0; i < *cx; i++) {
            CURSOR_Y = dh;
            CURSOR_X = dl + i;
            uint8_t character = *memory_at8(memory, *bp + i);
            addch_buffer(character | (al << 8));
        }
        CURSOR_Y = y;
        CURSOR_X = x;
        break;
    case 0x40:
        // Hercules Graphix - Set graphics mode
        // Not implemented
        empty_buffer();
        break;
    case 0x41:
        // Hercules Graphix - Set text mode
        // Not implemented
        empty_buffer();
        break;
    case 0x42:
        // Hercules Graphix - Clear current page
        empty_buffer();
        break;
    case 0x43:
        // Hercules Graphix - Select drawing page
        // AL = page number
        DRAWING_PAGE = al;
        break;
    case 0x44:
        // Hercules Graphix - Select drawing function
        // AL = drawing function (00h = clear pixels, 01h = set pixels, 02h = invert pixels)
        DRAWING_FUNCTION = al;
        break;
    case 0x45:
        // Hercules Graphix - Select page to display
        // AL = page number
        DISPLAY_PAGE = al;
        break;
    case 0x46:
        // Hercules Graphix - Draw one pixel
        // DI = x
        // BP = y
        draw_pixel_buffer(*bp, *di);
        break;
    case 0x47:
        // Hercules Graphix - Find pixel value
        // DI = x
        // BP = y
        // Returns AL = pixel value (00h = pixel clear, 01h = pixel set)
        *ax |= get_pixel_buffer(DRAWING_PAGE, *bp, *di) & 0xFF;
        break;
    case 0x48:
        // Hercules Graphix - Move to point
        // DI = x
        // BP = y
        PEN_X = *di;
        PEN_Y = *bp;
        break;
    case 0x49:
        // Hercules Graphix - Draw to point
        // DI = x
        // BP = y
        // CX = length
        
        // Draw a line from (PEN_X, PEN_Y) to (DI, BP)
        {
            int dx = *di - PEN_X;
            int dy = *bp - PEN_Y;
            int steps = sqrt(dx*dx + dy*dy);
            for (int i = 0; i < steps; i++) {
                int x = PEN_X + dx * i / steps;
                int y = PEN_Y + dy * i / steps;
                draw_pixel_buffer(y, x);
            }
        }
        break;
    case 0x4A:
        // Hercules Graphix - Block fill
        // DI = x (lower left corner)
        // BP = y (lower left corner)
        // BX = height
        // CX = width
        {
            int x = *di;
            int y = *bp;
            int height = *bx;
            int width = *cx;
            for (int i = y; i < y + height; i++) {
                for (int j = x; j < x + width; j++) {
                    draw_pixel_buffer(i, j);
                }
            }
        }
        break;
    case 0x4B:
        // Hercules Graphix - Draw character
        // DI = x (in pixels!)
        // BP = y (in pixels!)
        // AL = character
        // TODO: Not implemented
        break;
    case 0x4C:
        // Hercules Graphix - Draw arc
        // AL = quadrant (1 = upper right, 2 = upper left, etc)
        // DI = x coordinate of center
        // BP = y coordinate of center
        // BX = radius
        {
            int x = *di;
            int y = *bp;
            int radius = *bx;
            int quadrant = al;
            int start_angle = 0;
            //int end_angle = 0;
            switch (quadrant) {
            case 1:
                start_angle = 0;
                //end_angle = 90;
                break;
            case 2:
                start_angle = 90;
                //end_angle = 180;
                break;
            case 3:
                start_angle = 180;
                //end_angle = 270;
                break;
            case 4:
                start_angle = 270;
                //end_angle = 360;
                break;
            }
            int steps = 2 * M_PI * radius / 4;
            for (int i = 0; i < steps; i++) {
                int angle = i * 360 / steps + start_angle;
                int x1 = x + radius * cos(angle * M_PI / 180);
                int y1 = y + radius * sin(angle * M_PI / 180);
                draw_pixel_buffer(y1, x1);
            }
        }
        break;
    case 0x4D:
        // Hercules Graphix - Draw circle
        // DI = x coordinate of center
        // BP = y coordinate of center
        // BX = radius
        {
            int x = *di;
            int y = *bp;
            int radius = *bx;
            int steps = 2 * M_PI * radius;
            for (int i = 0; i < steps; i++) {
                int angle = i * 360 / steps;
                int x1 = x + radius * cos(angle * M_PI / 180);
                int y1 = y + radius * sin(angle * M_PI / 180);
                draw_pixel_buffer(y1, x1);
            }
        }
        break;
    case 0x4E:
        // Hercules Graphix - Fill area
        // DI = x coordinate of an interior point
        // BP = y coordinate of an interior point
        // TODO: Not implemented
        break;
    default:
        fprintf(stderr, "error: unhandled INT 10h function: AH = 0x%02X\n", ah);
        print_stack_trace(stderr);
        break;
    }
    if (OUTPUT_MODE == OUTPUT_MODE_CURSES) {
        redraw_buffer();
    }
}

void forth_linemove(int source, int dest) {
    // TODO: Figure out Herculer memory mapping
}
