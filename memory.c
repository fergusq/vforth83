#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include "errors.h"
#include "memory.h"
#include "util.h"

Memory *create_memory() {
    Memory *memory = malloc(sizeof(*memory));
    memory->memory_pointer = 1;
    memory->data_stack_size = 0;
    memory->return_stack_size = 0;
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
 * | name_length (1 byte) | name (max 27) | is_immediate (1) | previous_p (2) | type (1) | code_p (2) | parameter field (0) |
 * 
 * Returns the pointer to the parameter field. Sets latest_definition_p to point to the name_length field.
 * 
 * The caller should then allot something to the parameter field.
 */
uint16_t add_definition(Memory *memory, char *name, uint8_t is_immediate, enum DefinitionType type, uint8_t append_to_vocabulary) {
    uint16_t name_length = strlen(name);
    if (name_length > MAX_NAME_LENGTH) {
        name_length = MAX_NAME_LENGTH;
    }
    uint16_t previous_p = *memory_at16(memory, *memory->CURRENT_var);
    //printf("Defining %s at %i prev %i\n", name, memory->memory_pointer, previous_p);
    uint16_t nfa = insert8(memory, name_length);
    for (int i = 0; i < name_length; i++) {
        insert8(memory, name[i]);
    }
    insert8(memory, name_length);
    insert8(memory, is_immediate);
    insert16(memory, previous_p);
    insert16(memory, type);
    uint16_t pfa = insert16(memory, 0) + 2;

    *memory->LAST_var = nfa;
    if (append_to_vocabulary) {
        *memory_at16(memory, *memory->CURRENT_var) = nfa;
    }
    return pfa;
}

Definition *get_definition(Memory *memory, uint16_t nfa) {
    uint16_t cfa = FROM_NAME(memory, nfa);
    Definition *definition = malloc(sizeof(*definition));
    definition->name_length = *memory_at8(memory, nfa);
    definition->name = malloc(definition->name_length + 1);
    for (int i = 0; i < definition->name_length; i++) {
        definition->name[i] = *memory_at8(memory, nfa + i + 1);
    }
    definition->name[definition->name_length] = '\0';
    definition->is_immediate = *memory_at8(memory, TO_IMMEDIATE_FLAG(cfa));
    definition->previous_p = *memory_at16(memory, TO_LINK(cfa));
    definition->type = *memory_at16(memory, cfa);
    definition->code_p = *memory_at16(memory, TO_CODE_P(cfa));
    definition->pfa = TO_BODY(cfa);
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
            //printf("Searching for %s in %d, looking at %i %s %i next %i\n", name, i, p, definition->name, definition->pfa, definition->previous_p);
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

#define IMPLEMENT_STACK(nameupper, name, bottom_var, min_sp) int push_ ## name ## _stack(Memory *memory, uint16_t value) {\
    uint16_t size = memory->name ## _stack_size + 1;\
    uint16_t sp = *memory->bottom_var - 2*size + 2;\
    if (sp <= min_sp) {\
        return ERROR_ ## nameupper ## _STACK_OVERFLOW;\
    }\
    memory->name ## _stack_size++;\
    *memory_at16(memory, sp) = value;\
    return 0;\
}\
int pick_ ## name ## _stack(Memory *memory, uint16_t index, uint16_t *ret_val) {\
    uint16_t size = memory->name ## _stack_size;\
    if (index >= size) {\
        *ret_val = 0;\
        return 0;\
        return ERROR_ ## nameupper ## _STACK_OVERFLOW;\
    }\
    uint16_t sp = *memory->bottom_var - 2*size + 2 + 2*index;\
    *ret_val = *memory_at16(memory, sp);\
    return 0;\
}\
int pop_ ## name ## _stack(Memory *memory, uint16_t *ret_val) {\
    uint16_t size = memory->name ## _stack_size;\
    if (size == 0) {\
        return ERROR_ ## nameupper ## _STACK_UNDERFLOW;\
    }\
    uint16_t sp = *memory->bottom_var - 2*size + 2;\
    *ret_val = *memory_at16(memory, sp);\
    memory->name ## _stack_size--;\
    return 0;\
}

IMPLEMENT_STACK(DATA, data, SP0_var, memory->memory_pointer + 80)

IMPLEMENT_STACK(RETURN, return, RP0_var, *memory->SP0_var + 2)
