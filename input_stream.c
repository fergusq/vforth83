#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "forth.h"
#include "input_stream.h"
#include "errors.h"

int read_line_to_input_buffer(InterpreterState *state) {
    int err = read_line_to_input_buffer_from_file(state, stdin);
    if (err != 0) return err;
    printf("\033[1F\033[%dC ", *state->NUMBER_TIB_var);
    return 0;
}

int read_line_to_input_buffer_from_file(InterpreterState *state, FILE *file) {
    *state->NUMBER_TIB_var = 0;
    *state->TO_IN_var = 0;
    int c;
    while ((c = fgetc(file)) != '\n' && c != EOF) {
        if (*state->NUMBER_TIB_var >= MAX_INPUT_SIZE) {
            return ERROR_FILE_TOO_LARGE;
        }
        state->INPUT_BUFFER[*state->NUMBER_TIB_var] = c;
        *state->NUMBER_TIB_var += 1;
    }
    memset(&state->INPUT_BUFFER[*state->NUMBER_TIB_var], ' ', MAX_INPUT_SIZE - *state->NUMBER_TIB_var);
    if (c == EOF) {
        return ERROR_END_OF_INPUT;
    }
    return 0;
}

int read_paragraph_to_input_buffer_from_file(InterpreterState *state, FILE *file) {
    // Reads until empty line
    *state->NUMBER_TIB_var = 0;
    *state->TO_IN_var = 0;
    int c, p;
    while (c != EOF) {
        if (*state->NUMBER_TIB_var >= MAX_INPUT_SIZE) {
            return ERROR_FILE_TOO_LARGE;
        }
        c = fgetc(file);
        if (c == '\n' && p == '\n') break;
        state->INPUT_BUFFER[*state->NUMBER_TIB_var] = isspace(c) ? ' ' : c;
        *state->NUMBER_TIB_var += 1;
        p = c;
    }
    memset(&state->INPUT_BUFFER[*state->NUMBER_TIB_var], ' ', MAX_INPUT_SIZE - *state->NUMBER_TIB_var);
    if (c == EOF) {
        return ERROR_END_OF_INPUT;
    }
    return 0;
}

int read_string_to_input_buffer(InterpreterState *state, char *string) {
    *state->NUMBER_TIB_var = 0;
    *state->TO_IN_var = 0;
    int c;
    for (int i = 0; (c = string[i]) != 0; i++) {
        if (*state->NUMBER_TIB_var >= MAX_INPUT_SIZE) {
            return ERROR_FILE_TOO_LARGE;
        }
        state->INPUT_BUFFER[*state->NUMBER_TIB_var] = c;
        *state->NUMBER_TIB_var += 1;
    }
    memset(&state->INPUT_BUFFER[*state->NUMBER_TIB_var], ' ', MAX_INPUT_SIZE - *state->NUMBER_TIB_var);
    if (c == EOF) {
        return ERROR_END_OF_INPUT;
    }
    return 0;
}

uint8_t read_char(InterpreterState *state) {
    /*if (*state->BLK_var > 0 && *state->TO_IN_var < BLOCK_SIZE) {
        uint8_t c = state->BLOCK_BUFFER[*state->TO_IN_var];
        *state->TO_IN_var += 1;
        return c;
    } else*/ {
        //if (TIB == 0) {
        //    read_line_to_input_buffer();
        //}
        if (*state->NUMBER_TIB_var == 0 || *state->TO_IN_var >= *state->NUMBER_TIB_var) {
            return 0;
        } else {
            uint8_t c = state->INPUT_BUFFER[*state->TO_IN_var];
            *state->TO_IN_var += 1;
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
        while (i == 0 && isspace(c)) {
            c = read_char(state);
        }
        if (c == 0 || isspace(c)) {
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