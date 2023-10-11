#ifndef _INPUT_STREAM_H
#define _INPUT_STREAM_H

#include <stdint.h>
#include "forth.h"

int read_line_to_input_buffer(InterpreterState *state);

int read_line_to_input_buffer_from_file(InterpreterState *state, FILE *file);

uint8_t read_char(InterpreterState *state);

uint8_t *read_word(InterpreterState *state);

#endif