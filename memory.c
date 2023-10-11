#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "util.h"

Memory *create_memory() {
    Memory *memory = malloc(sizeof(*memory));
    memory->memory_pointer = 1;
    return memory;
}

void free_memory(Memory *memory) {
    free(memory);
}

uint16_t allot(Memory *memory, int16_t n) {
    if (memory->memory_pointer + n > MEMORY_SIZE) {
        return 0;
    } else {
        memory->memory_pointer += n;
        return memory->memory_pointer - n;
    }
}

uint16_t insert8(Memory *memory, uint8_t value) {
    uint16_t pointer;
    if ((pointer = allot(memory, 1)) == 0) return 0;
    *memory_at8(memory, pointer) = value;
    return pointer;
}

uint16_t insert16(Memory *memory, uint16_t value) {
    uint16_t pointer;
    if ((pointer = allot(memory, 2)) == 0) return 0;
    *memory_at16(memory, pointer) = value;
    return pointer;
}

/**
 * Inserts the definition into the memory with the following schema:
 * 
 * | name | name_length | is_immediate | previous_p | type | parameter field |
 * 
 * Returns the pointer to the parameter field. Sets latest_definition_p to point to the name_length field.
 * 
 * The caller should then allot something to the parameter field.
 */
uint16_t add_definition(Memory *memory, char *name, uint8_t is_immediate, enum DefinitionType type, uint8_t append_to_vocabulary) {
    uint16_t name_length = strlen(name);
    uint16_t previous_p = *memory_at16(memory, *memory->CURRENT_var);
    //printf("Defining %s at %i prev %i\n", name, memory->memory_pointer, previous_p);
    for (int i = 0; i < name_length; i++) {
        insert8(memory, name[i]);
    }
    uint16_t head_p = insert16(memory, name_length);
    insert8(memory, is_immediate);
    insert16(memory, previous_p);
    insert8(memory, type);
    uint16_t parameter_p = insert16(memory, 0) + 2;

    *memory->LAST_var = head_p;
    if (append_to_vocabulary) {
        *memory_at16(memory, *memory->CURRENT_var) = head_p;
    }
    return parameter_p;
}

Definition *get_definition(Memory *memory, uint16_t p) {
    Definition *definition = malloc(sizeof(*definition));
    definition->name_length = *memory_at16(memory, p);
    definition->name = malloc(definition->name_length + 1);
    for (int i = 0; i < definition->name_length; i++) {
        definition->name[i] = *memory_at8(memory, p - definition->name_length + i);
    }
    definition->name[definition->name_length] = '\0';
    definition->is_immediate = *memory_at8(memory, p + IS_IMMEDIATE_OFFSET);
    definition->previous_p = *memory_at16(memory, p + PREVIOUS_P_OFFSET);
    definition->type = *memory_at8(memory, p + TYPE_OFFSET);
    definition->code_p = *memory_at16(memory, p + CODE_P_OFFSET);
    definition->parameter_p =  p + PARAMETER_P_OFFSET;
    return definition;
}

Definition *find_word(Memory *memory, uint8_t *name) {
    uint8_t *normalized_name = upper(name);
    uint16_t *context = memory->CONTEXT_var;
    for (int i = 0; i < NUM_VOCS; i++) {
        if (context[i] == 0) {
            continue;
        }
        uint16_t p = *memory_at16(memory, context[i]);
        while (p != 0) {
            Definition *definition = get_definition(memory, p);
            //printf("Searching for %s, looking at %i %s %i next %i\n", name, p, definition->name, definition->parameter_p, definition->previous_p);
            if (strcmp(definition->name, name) == 0) {
                free(normalized_name);
                return definition;
            }
            p = definition->previous_p;
        }
    }
    free(normalized_name);
    return 0;
}

void free_definition(Definition *definition) {
    free(definition->name);
    free(definition);
}