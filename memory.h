#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

#define memory_at16(mem, p) ((uint16_t*) (((mem)->memory) + (p)))
#define memory_at8(mem, p) ((uint8_t*) (((mem)->memory) + (p)))

enum DefinitionType {
    DEFINITION_TYPE_BUILTIN = 0,
    DEFINITION_TYPE_VARIABLE,
    DEFINITION_TYPE_CONSTANT,
    DEFINITION_TYPE_CALL = 0x123,
    DEFINITION_TYPE_DOES,
    DEFINITION_TYPE_DEFERRED,
};

typedef struct _Definition {
    // Name field
    uint8_t name_length;
    char *name;
    uint8_t is_immediate;

    // Link field
    uint16_t previous_p; // pointer within virtual memory

    // Code field
    enum DefinitionType type;
    uint16_t code_p; // pointer within virtual memory, used for does> definitions

    // Parameter field
    uint16_t pfa; // pointer within virtual memory
} Definition;

#define MAX_NAME_LENGTH 25
#define TO_BODY(p) ((p) + 4)
#define FROM_BODY(p) ((p) - 4)
#define TO_LINK(p) ((p) - 2)
#define FROM_LINK(p) ((p) + 2)
#define TO_IMMEDIATE_FLAG(p) ((p) - 3)
#define FROM_IMMEDIATE_FLAG(p) ((p) + 3)
#define TO_NAME(mem, p) (TO_IMMEDIATE_FLAG(p) - 2 - *memory_at8(mem, TO_IMMEDIATE_FLAG(p) - 1))
#define FROM_NAME(mem, p) FROM_IMMEDIATE_FLAG(p + 2 + *memory_at8(mem, p))
#define TO_CODE_P(p) ((p) + 2)
#define FROM_CODE_P(p) ((p) - 2)

#define MEMORY_SIZE 65536
#define NUM_VOCS 8

typedef struct _Memory {
    uint16_t memory_pointer;
    uint8_t memory[MEMORY_SIZE];

    uint16_t *LAST_var;
    uint16_t *CONTEXT_var;
    uint16_t *CURRENT_var;
    uint16_t *SP0_var;
    uint16_t *RP0_var;

    uint16_t data_stack_size;
    uint16_t return_stack_size;
} Memory;

Memory *create_memory();

void free_memory(Memory *memory);

uint16_t allot(Memory *memory, int16_t n);

uint16_t insert8(Memory *memory, uint8_t value);
uint16_t insert16(Memory *memory, uint16_t value);

uint16_t add_definition(Memory *memory, char *name, uint8_t is_immediate, enum DefinitionType type, uint8_t append_to_vocabulary);

Definition *get_definition(Memory *memory, uint16_t p);

Definition *find_word(Memory *memory, uint8_t *name);

void free_definition(Definition *definition);

int push_data_stack(Memory *memory, uint16_t value);

int pick_data_stack(Memory *memory, uint16_t index, uint16_t *ret_val);

int pop_data_stack(Memory *memory, uint16_t *ret_val);

int push_return_stack(Memory *memory, uint16_t value);

int pick_return_stack(Memory *memory, uint16_t index, uint16_t *ret_val);

int pop_return_stack(Memory *memory, uint16_t *ret_val);

#endif