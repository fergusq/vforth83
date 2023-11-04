#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include <stdint.h>
#include "forth.h"

int read_line_to_input_buffer(InterpreterState *state);

int read_line_to_input_buffer_from_file(InterpreterState *state, FILE *file);

int read_paragraph_to_input_buffer_from_file(InterpreterState *state, FILE *file);

int read_string_to_input_buffer(InterpreterState *state, char *string);

uint8_t read_char(InterpreterState *state);

uint8_t *read_word(InterpreterState *state);

#endif