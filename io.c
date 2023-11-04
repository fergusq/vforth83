#define _XOPEN_SOURCE_EXTENDED

#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/io.h>
#include <termio.h>
#include <unistd.h>
#include <wchar.h>
#include <ncursesw/curses.h>

#include "forth.h"
#include "io.h"
#include "memory.h"

wchar_t *CP850_TO_UTF8[] = {
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

uint8_t IS_INITIALIZED = 0;
enum InputMode INPUT_MODE = 0;

WINDOW* OUTPUT_WINDOW;

void init_io() {
    if (IS_INITIALIZED) return;
    IS_INITIALIZED = 1;
    INPUT_MODE = INPUT_MODE_NO_ECHO;
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

}

void free_io() {
    delwin(OUTPUT_WINDOW);
    endwin();
}

void forth_getyx(int *y, int *x) {
    getyx(OUTPUT_WINDOW, *y, *x);
}

char OUTPUT_BUFFER[80];

void forth_printf(const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int size = vsnprintf(OUTPUT_BUFFER, 80, format, argptr);
    if (size >= 0) {
        if (IS_INITIALIZED) {
            wprintw(OUTPUT_WINDOW, "%s", OUTPUT_BUFFER);
            wrefresh(OUTPUT_WINDOW);
        } else {
            printf("%s", OUTPUT_BUFFER);
        }
    }
    va_end(argptr);
}

void forth_addch(char character) {
    if (IS_INITIALIZED) {
        waddch(OUTPUT_WINDOW, character);
        wrefresh(OUTPUT_WINDOW);
    } else {
        putchar(character);
    }
}

int buffer = 0;

int forth_getch() {
    if (IS_INITIALIZED) {
        if (buffer != 0) {
            int result = buffer;
            buffer = 0;
            return result;
        }
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
    } else {
        return getchar();
    }
}

int forth_kbhit() {
    if (buffer != 0) {
        return 1;
    }

    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}

void copyline(int sourcerow, int sourcecol, int sourcelen, int destrow, int destcol) {
    char line[sourcelen+1];
    mvwinnstr(OUTPUT_WINDOW, sourcerow, sourcecol, line, sourcelen);
    mvwinstr(OUTPUT_WINDOW, destrow, destcol, line);
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
    if (0) {
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
        werase(OUTPUT_WINDOW);
        break;
    case 0x01:
        // Set cursor type
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x02:
        // Set cursor position
        // DH = Row
        // DL = Column
        wmove(OUTPUT_WINDOW, dh, dl);
        wrefresh(OUTPUT_WINDOW);
        break;
    case 0x03:
        // Get cursor position and size
        getyx(OUTPUT_WINDOW, y, x);
        *dx = ((y & 0xFF) << 8) | (x & 0xFF);
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
        getyx(OUTPUT_WINDOW, y, x);
        if (al == 0) {
            /*for (int i = ch; i < dh; i++) {
                for (int j = cl; j < dl; j++) {
                    mvwinsch(OUTPUT_WINDOW, i, j, ' ' | (chtype)bh << 8);
                }
            }*/
            werase(OUTPUT_WINDOW);
        } else {
            for (int i = al; i < dh-ch; i++) {
                copyline(ch+i, cl, dl-cl+1, ch+i-al, cl);
            }
            wmove(OUTPUT_WINDOW, dh, cl);
            for (int i = 0; i < dl-cl+1; i++) {
                waddch(OUTPUT_WINDOW, ' ' | (chtype)bh << 8);
            }
        }
        wmove(OUTPUT_WINDOW, y, x);
        break;
    case 0x07:
        // Scroll down window
        // AL = number of lines by which to scroll down (00h = clear entire window)
        // BH = attribute used to write blank lines at bottom of window
        // CH,CL = row,column of window's upper left corner
        // DH,DL = row,column of window's lower right corner
        getyx(OUTPUT_WINDOW, y, x);
        if (al == 0) {
            /*for (int i = ch; i < dh; i++) {
                for (int j = cl; j < dl; j++) {
                    mvwinsch(OUTPUT_WINDOW, i, j, ' ' | (chtype)bh << 8);
                }
            }*/
            werase(OUTPUT_WINDOW);
        } else {
            for (int i = al; i < dh-ch; i++) {
                copyline(dh-i, cl, dl-cl+1, dh-i+al, cl);
            }
            wmove(OUTPUT_WINDOW, dh, cl);
            for (int i = 0; i < dl-cl+1; i++) {
                waddch(OUTPUT_WINDOW, ' ' | (chtype)bh << 8);
            }
        }
        wmove(OUTPUT_WINDOW, y, x);
        break;
    case 0x08:
        // Read character and attribute at cursor
        chtype ch = winch(OUTPUT_WINDOW);
        *ax = ((ch & 0xFF00) << 8) | (ch & 0xFF);
        break;
    case 0x09:
        // Write character and attribute at cursor
        // AL = Character
        // BH = Page
        // BL = Attribute
        // CX = Number of times to write
        getyx(OUTPUT_WINDOW, y, x);
        for (uint16_t i = 0; i < *cx; i++) {
            if (al >= 0x80) {
                cchar_t ch = { 0 };
                setcchar(&ch, CP850_TO_UTF8[al], WA_NORMAL, 0, NULL);
                wadd_wch(OUTPUT_WINDOW, &ch); // TODO attributes
            } else {
                waddch(OUTPUT_WINDOW, (chtype)al | (chtype)bl << 8);
            }
        }
        wmove(OUTPUT_WINDOW, y, x);
        break;
    case 0x0A:
        // Write character at cursor
        // AL = Character
        // BH = Page
        // CX = Number of times to write
        getyx(OUTPUT_WINDOW, y, x);
        for (uint16_t i = 0; i < *cx; i++) {
            if (al >= 0x80) {
                cchar_t ch = { 0 };
                setcchar(&ch, CP850_TO_UTF8[al], WA_NORMAL, 0, NULL);
                wadd_wch(OUTPUT_WINDOW, &ch);
            } else {
                waddch(OUTPUT_WINDOW, al);
            }
        }
        wmove(OUTPUT_WINDOW, y, x);
        break;
    case 0x0B:
        // Set color palette
        // Not implemented
        break;
    case 0x0C:
        // Write graphics pixel
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x0D:
        // Read graphics pixel
        // Not implemented since graphics not possible with NCurses
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
        getyx(OUTPUT_WINDOW, y, x);
        for (uint16_t i = 0; i < *cx; i++) {
            wmove(OUTPUT_WINDOW, dh, dl + i);
            uint8_t character = *memory_at8(memory, *bp + i);
            if (character >= 0x80) {
                cchar_t ch = { 0 };
                setcchar(&ch, CP850_TO_UTF8[character], WA_NORMAL, 0, NULL);
                wadd_wch(OUTPUT_WINDOW, &ch); // TODO attributes
            } else {
                waddch(OUTPUT_WINDOW, character | (chtype)al << 8);
            }
        }
        wmove(OUTPUT_WINDOW, y, x);
        break;
    case 0x40:
        // Hercules Graphix - Set graphics mode
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x41:
        // Hercules Graphix - Set text mode
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x42:
        // Hercules Graphix - Clear current page
        werase(OUTPUT_WINDOW);
        break;
    case 0x43:
        // Hercules Graphix - Select drawing page
        // AL = page number
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x44:
        // Hercules Graphix - Select drawing function
        // AL = drawing function (00h = clear pixels, 01h = set pixels, 02h = invert pixels)
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x45:
        // Hercules Graphix - Select page to display
        // AL = page number
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x46:
        // Hercules Graphix - Draw one pixel
        // DI = x
        // BP = y
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x47:
        // Hercules Graphix - Find pixel value
        // DI = x
        // BP = y
        // Returns AL = pixel value (00h = pixel clear, 01h = pixel set)
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x48:
        // Hercules Graphix - Move to point
        // DI = x
        // BP = y
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x49:
        // Hercules Graphix - Draw to point
        // DI = x
        // BP = y
        // CX = length
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x4A:
        // Hercules Graphix - Block fill
        // DI = x (lower left corner)
        // BP = y (lower left corner)
        // BX = height
        // CX = width
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x4B:
        // Hercules Graphix - Draw character
        // DI = x (in pixels!)
        // BP = y (in pixels!)
        // AL = character
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x4C:
        // Hercules Graphix - Draw arc
        // AL = quadrant (1 = upper right, 2 = upper left, etc)
        // DI = x coordinate of center
        // BP = y coordinate of center
        // BX = radius
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x4D:
        // Hercules Graphix - Draw circle
        // DI = x coordinate of center
        // BP = y coordinate of center
        // BX = radius
        // Not implemented since graphics not possible with NCurses
        break;
    case 0x4E:
        // Hercules Graphix - Fill area
        // DI = x coordinate of an interior point
        // BP = y coordinate of an interior point
        // Not implemented since graphics not possible with NCurses
        break;
    default:
        fprintf(stderr, "error: unhandled INT 10h function: AH = 0x%02X\n", ah);
        print_stack_trace(stderr);
        break;
    }
}

void forth_linemove(int source, int dest) {
    // TODO: Figure out Herculer memory mapping
}
