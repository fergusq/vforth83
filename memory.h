#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>

enum DefinitionType {
    DEFINITION_TYPE_BUILTIN = 0,
    DEFINITION_TYPE_COLON = 1,
    DEFINITION_TYPE_VARIABLE = 2,
    DEFINITION_TYPE_CONSTANT = 3,
    DEFINITION_TYPE_DOES = 4,
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
    uint16_t parameter_p; // pointer within virtual memory
} Definition;

#define IS_IMMEDIATE_OFFSET 2
#define PREVIOUS_P_OFFSET 3
#define TYPE_OFFSET 5
#define CODE_P_OFFSET 6
#define PARAMETER_P_OFFSET 8

#define MEMORY_SIZE 65536

typedef struct _Memory {
    uint16_t memory_pointer;
    uint8_t memory[MEMORY_SIZE];

    uint16_t latest_definition_p; // pointer within virtual memory
} Memory;

#define memory_at16(mem, p) ((uint16_t*) (((mem)->memory) + (p)))
#define memory_at8(mem, p) ((uint8_t*) (((mem)->memory) + (p)))

Memory *create_memory();

void free_memory(Memory *memory);

uint16_t allot(Memory *memory, int16_t n);

uint16_t insert8(Memory *memory, uint8_t value);
uint16_t insert16(Memory *memory, uint16_t value);

uint16_t add_definition(Memory *memory, char *name, uint8_t is_immediate, enum DefinitionType type, uint8_t set_latest);

Definition *get_definition(Memory *memory, uint16_t p);

Definition *find_word(Memory *memory, char *name);

void free_definition(Definition *definition);

#endif