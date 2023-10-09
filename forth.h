#ifndef _FORTH_H
#define _FORTH_H

#include <stdint.h>

typedef uint16_t ForthValue;

#include "memory.h"
#include "stack.h"

typedef struct _InterpreterState {
    Stack *RETURN_STACK;
    Stack *DATA_STACK;

    Memory *MEMORY;

    // Input buffers

    uint8_t BLOCK_BUFFER[1024];
    uint16_t BLK;

    #define MAX_INPUT_SIZE 1024

    uint8_t INPUT_BUFFER[MAX_INPUT_SIZE];
    uint16_t TIB;
    uint16_t TO_IN;

    // Compiler state

    uint8_t compilation_flag;
    uint16_t program_counter;

    // Builtin compilation addresses

    uint16_t BUILTINS[200];
} InterpreterState;

#include "builtins.h"

void init_forth();

void free_forth();

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate);

void create_forth_vocabulary();

void interpret_from_input_stream();

void interpret_from_memory();

int execute_word(uint16_t p);


#endif