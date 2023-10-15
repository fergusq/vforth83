#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

enum DefinitionType {
    DEFINITION_TYPE_BUILTIN = 0,
    DEFINITION_TYPE_VARIABLE,
    DEFINITION_TYPE_CONSTANT,
    DEFINITION_TYPE_CALL,
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

//#define IS_IMMEDIATE_OFFSET 2
//#define PREVIOUS_P_OFFSET 3
//#define TYPE_OFFSET 5
//#define CODE_P_OFFSET 6
//#define PARAMETER_P_OFFSET 8

#define MAX_NAME_LENGTH 25
#define TO_BODY(p) ((p) + 3)
#define FROM_BODY(p) ((p) - 3)
#define TO_LINK(p) ((p) - 2)
#define FROM_LINK(p) ((p) + 2)
#define TO_IMMEDIATE_FLAG(p) ((p) - 3)
#define FROM_IMMEDIATE_FLAG(p) ((p) + 3)
#define TO_NAME(p) TO_IMMEDIATE_FLAG(p) - MAX_NAME_LENGTH - 1
#define FROM_NAME(p) FROM_IMMEDIATE_FLAG(p + MAX_NAME_LENGTH + 1)
#define TO_CODE_P(p) ((p) + 1)
#define FROM_CODE_P(p) ((p) - 1)

#define MEMORY_SIZE 65536
#define NUM_VOCS 8

typedef struct _Memory {
    uint16_t memory_pointer;
    uint8_t memory[MEMORY_SIZE];

    uint16_t *LAST_var;
    uint16_t *CONTEXT_var;
    uint16_t *CURRENT_var;
} Memory;

#define memory_at16(mem, p) ((uint16_t*) (((mem)->memory) + (p)))
#define memory_at8(mem, p) ((uint8_t*) (((mem)->memory) + (p)))

Memory *create_memory();

void free_memory(Memory *memory);

uint16_t allot(Memory *memory, int16_t n);

uint16_t insert8(Memory *memory, uint8_t value);
uint16_t insert16(Memory *memory, uint16_t value);

uint16_t add_definition(Memory *memory, char *name, uint8_t is_immediate, enum DefinitionType type, uint8_t append_to_vocabulary);

Definition *get_definition(Memory *memory, uint16_t p);

Definition *find_word(Memory *memory, uint8_t *name);

void free_definition(Definition *definition);

#endif