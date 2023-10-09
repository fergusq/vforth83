#ifndef _INPUT_STREAM_H
#define _INPUT_STREAM_H

#include <stdint.h>
#include "forth.h"

void read_line_to_input_buffer(InterpreterState *state);

uint8_t read_char(InterpreterState *state);

uint8_t *read_word(InterpreterState *state);

#endif