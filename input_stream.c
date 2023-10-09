#include <malloc.h>
#include "forth.h"
#include "input_stream.h"

void read_line_to_input_buffer(InterpreterState *state) {
    state->TIB = 0;
    state->TO_IN = 0;
    int c;
    while (state->TIB < MAX_INPUT_SIZE && (c = getchar()) != '\n') {
        state->INPUT_BUFFER[state->TIB] = c;
        state->TIB += 1;
    }
}

uint8_t read_char(InterpreterState *state) {
    if (state->BLK > 0) {
        uint8_t c = state->BLOCK_BUFFER[state->BLK];
        state->BLK += 1;
        return c;
    } else {
        //if (TIB == 0) {
        //    read_line_to_input_buffer();
        //}
        if (state->TIB == 0 || state->TO_IN >= state->TIB) {
            return 0;
        } else {
            uint8_t c = state->INPUT_BUFFER[state->TO_IN];
            state->TO_IN += 1;
            return c;
        }
    }
}

uint8_t *read_word(InterpreterState *state) {
    int size = 8;
    uint8_t *word = malloc(size);
    int i = 0;
    while (1) {
        uint8_t c = read_char(state);
        while (i == 0 && (c == ' ' || c == '\n')) {
            c = read_char(state);
        }
        if (c == 0 || c == ' ' || c == '\n') {
            break;
        }
        if (i == size) {
            size *= 2;
            word = realloc(word, size);
        }
        word[i] = c;
        i += 1;
    }
    if (i == 0) {
        free(word);
        return 0;
    } else if (size > i) {
        word[i] = 0;
        return word;
    } else {
        word = realloc(word, i + 1);
        word[i] = 0;
        return word;
    }
}