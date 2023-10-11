#ifndef _FORTH_H
#define _FORTH_H

#include <stdint.h>

typedef uint16_t ForthValue;

#include "memory.h"
#include "stack.h"
#include "mass_storage.h"

#define MAX_INPUT_SIZE 1024

typedef struct _InterpreterState {
    Stack *RETURN_STACK;
    Stack *DATA_STACK;

    Memory *MEMORY;

    // Input buffers

    uint8_t BLOCK_BUFFER[BLOCK_SIZE];
    uint16_t *BLK_var;

    uint8_t *INPUT_BUFFER;
    uint16_t *NUMBER_TIB_var;
    uint16_t *TO_IN_var;

    // Misc variables

    uint16_t *BASE_var;
    uint16_t *SPAN_var;
    uint16_t *STATE_var;
    uint16_t *LAST_var;
    uint16_t *CURRENT_var;
    uint16_t *CONTEXT_var;
    uint16_t *VOC_LINK_var;
    uint16_t *FILE_var;

    // Compiler state

    uint16_t program_counter;

    // Builtin compilation addresses

    uint16_t BUILTINS[200];
} InterpreterState;

#include "builtins.h"

void init_forth();

void free_forth();

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate);

void create_forth_vocabulary();

int interpret_from_input_stream();

void interpret_from_memory();

int execute_word(uint16_t p);


#endif