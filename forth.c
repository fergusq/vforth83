#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "forth.h"
#include "stack.h"
#include "memory.h"
#include "builtins.h"
#include "input_stream.h"

// *** Memory ***

InterpreterState *state;

// *** Initialization and cleanup ***

void init_forth() {
    state = malloc(sizeof(*state));
    state->RETURN_STACK = create_stack();
    state->DATA_STACK = create_stack();
    state->MEMORY = create_memory();
    state->compilation_flag = 0;
    state->program_counter = 0;
    state->BLK = 0;
    state->TIB = 0;
    state->TO_IN = 0;
}

void free_forth() {
    free_stack(state->RETURN_STACK);
    free_stack(state->DATA_STACK);
    free_memory(state->MEMORY);
    free(state);
}

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate) {
    add_definition(state->MEMORY, name, is_immediate, DEFINITION_TYPE_BUILTIN, 1);
    uint16_t addr = insert16(state->MEMORY, word);
    state->BUILTINS[word] = addr;
    if (addr == 0) {
        printf("error while defining builtin %s\n", name);
    }
}

void create_forth_vocabulary() {
    add_builtins(add_builtin);
}

// *** Interpreter ***

// Forth has two interpreters: one that interprets words from the input stream, another that interprets compiled words from the memory

void interpret_from_input_stream() {
    while (1) {
        uint8_t *word = read_word(state);
        if (word == 0) {
            break;
        }
        if (strspn(word, "0123456789") == strlen(word)) {
            uint16_t num = atoi(word);
            if (state->compilation_flag) {
                insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
                insert16(state->MEMORY, num);
            } else {
                push(state->DATA_STACK, num);
            }
            free(word);
        } else {
            Definition *definition = find_word(state->MEMORY, word);
            if (definition == 0) {
                printf("%s?\n", word);
                free(word);
                break;
            }
            if (state->compilation_flag && !definition->is_immediate) {
                insert16(state->MEMORY, definition->parameter_p - PARAMETER_P_OFFSET + TYPE_OFFSET);
                free(word);
                free_definition(definition);
            } else {
                //printf("Executing %d\n", definition->parameter_p - PARAMETER_P_OFFSET + TYPE_OFFSET);
                int ret = execute_word(definition->parameter_p - PARAMETER_P_OFFSET + TYPE_OFFSET);
                free_definition(definition);
                if (ret == -1) {
                    free(word);
                    break;
                } else if (ret > 0) {
                    printf("error %d while executing word %s\n", ret, word);
                    free(word);
                    break;
                }
                free(word);
            }
        }
    }
}

void interpret_from_memory() {
    while (1) {
        uint16_t instruction = *memory_at16(state->MEMORY, state->program_counter);
        //printf("Executing %d at %d\n", instruction, state->program_counter);
        state->program_counter += 2;
        if (instruction == state->BUILTINS[BUILTIN_WORD_LIT]) {
            uint16_t num = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += 2;
            push(state->DATA_STACK, num);
        } else if (instruction == state->BUILTINS[BUILTIN_WORD_BRANCH]) {
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter = addr;
        } else if (instruction == state->BUILTINS[BUILTIN_WORD_QUESTION_BRANCH]) {
            uint16_t flag;
            if (pop(state->DATA_STACK, &flag) == -1) {
                printf("stack underflow\n");
                break;
            }
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            if (flag != 0) {
                state->program_counter = addr;
            } else {
                state->program_counter += 2;
            }
        } else {
            int ret = execute_word(instruction);
            if (ret == -1) {
                break;
            } else if (ret > 0) {
                printf("error %d while executing word %d\n", ret, instruction);
                break;
            }
        }
    }
}

// *** Execution ***

int execute_word(uint16_t p) {
    enum DefinitionType type = *memory_at8(state->MEMORY, p);
    if (type == DEFINITION_TYPE_VARIABLE) {
        // Variables push their parameter field address to the stack
        push(state->DATA_STACK, p - TYPE_OFFSET + PARAMETER_P_OFFSET);
        return 0;
    } else if (type == DEFINITION_TYPE_CONSTANT) {
        // Constants push their parameter field value to the stack
        push(state->DATA_STACK, *memory_at16(state->MEMORY, p - TYPE_OFFSET + PARAMETER_P_OFFSET));
        return 0;
    } else if (type == DEFINITION_TYPE_BUILTIN) {
        // Builtins are executed with their respective functions in builtins.c
        return execute_builtin(state, *memory_at16(state->MEMORY, p - TYPE_OFFSET + PARAMETER_P_OFFSET));
    } else if (type == DEFINITION_TYPE_COLON) {
        // Colon definitions are executed by moving the program counter to the parameter field
        push(state->RETURN_STACK, state->program_counter);
        state->program_counter = p + 1;
        
        // If we are not already executing a function, start executing it now
        if (state->RETURN_STACK->size == 1) {
            interpret_from_memory();
        }
        return 0;
    } else if (type == DEFINITION_TYPE_DOES) {
        // Does> definitions are executed by moving the program counter to the code field
        push(state->RETURN_STACK, state->program_counter);
        state->program_counter = *memory_at16(state->MEMORY, p - TYPE_OFFSET + CODE_P_OFFSET);

        // Address of the param field is pushed to the stack
        push(state->DATA_STACK, p - TYPE_OFFSET + PARAMETER_P_OFFSET);
        
        // If we are not already executing a function, start executing it now
        if (state->RETURN_STACK->size == 1) {
            interpret_from_memory();
        }
        return 0;
    } else {
        // This should never happen
        return 1;
    }
}

int main() {
    init_forth();
    create_forth_vocabulary();
    while (1) {
        read_line_to_input_buffer(state);
        interpret_from_input_stream();
        printf("ok\n");
    }
    free_forth();
}