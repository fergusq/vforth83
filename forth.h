#ifndef _FORTH_H
#define _FORTH_H

#include <stdint.h>

typedef uint16_t ForthValue;

#include "memory.h"
#include "stack.h"

#define MAX_INPUT_SIZE 1024

typedef struct _InterpreterState {
    Stack *DEBUG_CALL_STACK;

    Memory *MEMORY;

    // Input buffers

    uint16_t *BLK_var;

    uint8_t *INPUT_BUFFER;
    uint16_t *NUMBER_TIB_var;
    uint16_t *TO_IN_var;

    // Misc variables

    uint16_t *SP0_var;
    uint16_t *RP0_var;
    uint16_t *BASE_var;
    uint16_t *SPAN_var;
    uint16_t *STATE_var;
    uint16_t *LAST_var;
    uint16_t *CURRENT_var;
    uint16_t *CONTEXT_var;
    uint16_t *CAPS_var;
    uint16_t *VOC_LINK_var;

    // Bytecode interpreter state

    uint16_t program_counter;

    // Builtin compilation addresses

    uint16_t BUILTINS[256];
} InterpreterState;

extern uint8_t TRACE;

#include "builtins.h"

void init_forth();

void free_forth();

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate);

void create_forth_vocabulary();

int interpret_from_input_stream();

int interpret_from_memory();

int execute_word(uint16_t p);


#endif