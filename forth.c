#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "forth.h"
#include "stack.h"
#include "memory.h"
#include "builtins.h"
#include "input_stream.h"
#include "errors.h"
#include "util.h"

#define print_error(err) {\
        char *message = get_error_string(err);\
        printf("error: %s (%d)\n", message, err);\
    }

uint8_t TRACE = 0;

// *** Memory ***

InterpreterState *state;

// *** Initialization and cleanup ***

void init_forth() {
    state = malloc(sizeof(*state));
    state->RETURN_STACK = create_stack();
    state->DATA_STACK = create_stack();
    state->DEBUG_CALL_STACK = create_stack();
    state->MEMORY = create_memory();
    state->program_counter = 0;
}

void free_forth() {
    free_stack(state->RETURN_STACK);
    free_stack(state->DATA_STACK);
    free_stack(state->DEBUG_CALL_STACK);
    free_memory(state->MEMORY);
    free(state);
}

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate) {
    if (BUILTINS[word] == 0) {
        return;
    }
    add_definition(state->MEMORY, name, is_immediate, DEFINITION_TYPE_BUILTIN, 1);
    uint16_t addr = insert16(state->MEMORY, word);
    state->BUILTINS[word] = FROM_BODY(addr);
    if (addr == 0) {
        printf("error while defining builtin %s\n", name);
    }
}

uint16_t add_variable(char *name) {
    add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_VARIABLE, 1);
    return allot(state->MEMORY, 2);
}

uint16_t add_constant(char *name, uint16_t value) {
    add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_CONSTANT, 1);
    return insert16(state->MEMORY, value);
}

void create_forth_vocabulary() {

    // Bootstrap vocabulary-related variables and reserve the FORTH vocabulary

    state->MEMORY->LAST_var = state->LAST_var = memory_at16(state->MEMORY, allot(state->MEMORY, 2));
    state->MEMORY->CURRENT_var = state->CURRENT_var = memory_at16(state->MEMORY, allot(state->MEMORY, 2)); // Temporary space for CURRENT
    uint16_t FORTH_pfa = add_definition(state->MEMORY, "FORTH", 0, DEFINITION_TYPE_DOES, 0);
    insert16(state->MEMORY, TO_NAME(FROM_BODY(FORTH_pfa))); // Add the word FORTH as the root of the FORTH chain
    //*memory_at16(state->MEMORY, FORTH_pfa - PARAMETER_P_OFFSET + CODE_P_OFFSET) = 0;
    *state->MEMORY->CURRENT_var = FORTH_pfa;

    // Add builtins

    add_builtins(add_builtin);

    // Create variables

    state->BLK_var = memory_at16(state->MEMORY, add_variable("BLK"));
    state->NUMBER_TIB_var = memory_at16(state->MEMORY, add_variable("#TIB"));
    state->TO_IN_var = memory_at16(state->MEMORY, add_variable(">IN"));
    state->BASE_var = memory_at16(state->MEMORY, add_variable("BASE"));
    state->SPAN_var = memory_at16(state->MEMORY, add_variable("SPAN"));
    state->STATE_var = memory_at16(state->MEMORY, add_variable("STATE"));
    state->MEMORY->LAST_var = state->LAST_var = memory_at16(state->MEMORY, add_variable("LAST"));
    state->CURRENT_var = memory_at16(state->MEMORY, add_variable("CURRENT"));
    state->CAPS_var = memory_at16(state->MEMORY, add_variable("CAPS"));
    state->VOC_LINK_var = memory_at16(state->MEMORY, add_variable("VOC-LINK"));
    add_constant("#VOCS", NUM_VOCS);

    *state->BASE_var = 10;

    *state->CURRENT_var = *state->MEMORY->CURRENT_var;
    state->MEMORY->CURRENT_var = state->CURRENT_var;

    // Reserve input buffer

    add_definition(state->MEMORY, "TIB", 0, DEFINITION_TYPE_VARIABLE, 1);
    state->INPUT_BUFFER = memory_at8(state->MEMORY, allot(state->MEMORY, MAX_INPUT_SIZE));

    // Reseve search order

    add_definition(state->MEMORY, "CONTEXT", 0, DEFINITION_TYPE_VARIABLE, 1);
    state->MEMORY->CONTEXT_var = state->CONTEXT_var = memory_at16(state->MEMORY, allot(state->MEMORY, NUM_VOCS*2));
    *state->MEMORY->CONTEXT_var = *state->CURRENT_var;
    for (int i = 1; i < NUM_VOCS; i++) {
        state->MEMORY->CONTEXT_var[i] = 0;
    }
}

void execute_system() {
    FILE *file = fopen("system.f", "r");
    if (file == 0) {
        printf("error: could not open system.f\n");
        return;
    }

    // Read line by line and interpret
    while (1) {
        int err = read_line_to_input_buffer_from_file(state, file);
        if (err == ERROR_END_OF_INPUT) break;
        else if (err != 0) print_error(err);
        interpret_from_input_stream();
    }

    fclose(file);
}

// *** Debug ***

void print_stack_trace() {
    for (int i = state->DEBUG_CALL_STACK->size - 1; i >= 0; i--) {
        uint16_t cfa = state->DEBUG_CALL_STACK->bottom[i];
        Definition *dbg_call_stack_definition = get_definition(state->MEMORY, TO_NAME(cfa));
        printf(" < %s", dbg_call_stack_definition->name);
        free_definition(dbg_call_stack_definition);
    }
}

// *** Interpreter ***

// Forth has two interpreters: one that interprets words from the input stream, another that interprets compiled words from the memory

int interpret_from_input_stream() {
    int error = 0;
    while (1) {
        uint8_t *word = read_word(state);
        if (word == 0) {
            break;
        }
        if (*state->CAPS_var != 0) {
            uint8_t *word_upper = upper(word);
            free(word);
            word = word_upper;
        }
        //printf("Executing %s from input stream\n", word);
        Definition *definition = find_word(state->MEMORY, word);
        if (definition == 0) {
            // Try parsing it as a number
            int8_t sign = 1, index = 0, num_parsing_error = 0, len = strlen(word);
            uint16_t value = 0;
            if (word[0] == '-') { sign = -1; index += 1; }
            for (; index < len; index++) {
                uint8_t chr = word[index];
                if (chr > '9' && *state->BASE_var > 10) {
                    chr = toupper(chr) - 'A' + 10;
                } else {
                    chr = chr - '0';
                }
                if (chr > *state->BASE_var) {
                    num_parsing_error = 1;
                    break;
                }
                value = value * *state->BASE_var + chr;
            }
            if (num_parsing_error) {
                error = 1;
                printf("%s?\n", word);
                free(word);
                error = 1;
                break;
            } else {
                if (sign == -1) value |= 1 << 15;
                if (*state->STATE_var != 0) {
                    insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
                    insert16(state->MEMORY, value);
                } else {
                    push(state->DATA_STACK, value);
                }
                free(word);
                continue;
            }
        }
        if (*state->STATE_var != 0 && !definition->is_immediate) {
            insert16(state->MEMORY, FROM_BODY(definition->pfa));
            free(word);
            free_definition(definition);
        } else {
            //printf("Executing %d\n", FROM_BODY(definition->parameter_p));
            int ret = execute_word(FROM_BODY(definition->pfa));
            free_definition(definition);
            if (ret == -1) {
                free(word);
                break;
            } else if (ret > 0) {
                char *message = get_error_string(ret);
                printf("error: %s (%d) while executing word `%s'\n", message, ret, word);
                free(word);
                error = 1;
                break;
            }
            free(word);
        }
    }
    return error;
}

void interpret_from_memory() {
    while (1) {
        uint16_t cfa = *memory_at16(state->MEMORY, state->program_counter);
        if (TRACE) {
            Definition *definition = get_definition(state->MEMORY, TO_NAME(cfa));
            printf("Executing %d %s at %d", cfa, definition->name, state->program_counter);
            print_stack_trace();
            printf("\n");
            free_definition(definition);
        }
        state->program_counter += 2;
        if (cfa == state->BUILTINS[BUILTIN_WORD_LIT]) {
            uint16_t num = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += 2;
            push(state->DATA_STACK, num);
        } else if (cfa == state->BUILTINS[BUILTIN_WORD_BRANCH]) {
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += addr;
        } else if (cfa == state->BUILTINS[BUILTIN_WORD_QUESTION_BRANCH]) {
            uint16_t flag;
            if (pop(state->DATA_STACK, &flag) == -1) {
                printf("Stack Underflow\n");
                break;
            }
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            if (flag == 0) {
                state->program_counter += addr;
            } else {
                state->program_counter += 2;
            }
        } else {
            int ret = execute_word(cfa);
            if (ret == -1) {
                break;
            } else if (ret > 0) {
                Definition *definition = get_definition(state->MEMORY, TO_NAME(cfa));
                char *message = get_error_string(ret);
                printf("error: %s (%d) while executing word %u %s", message, ret, cfa, definition->name);
                print_stack_trace();
                printf("\n");
                free_definition(definition);
                break;
            }
        }
    }
}

// *** Execution ***

int execute_word(uint16_t cfa) {
    enum DefinitionType type = *memory_at8(state->MEMORY, cfa);
    if (type == DEFINITION_TYPE_VARIABLE) {
        // Variables push their parameter field address to the stack
        push(state->DATA_STACK, TO_BODY(cfa));
        return 0;
    } else if (type == DEFINITION_TYPE_CONSTANT) {
        // Constants push their parameter field value to the stack
        push(state->DATA_STACK, *memory_at16(state->MEMORY, TO_BODY(cfa)));
        return 0;
    } else if (type == DEFINITION_TYPE_BUILTIN) {
        // Builtins are executed with their respective functions in builtins.c
        return execute_builtin(state, *memory_at16(state->MEMORY, TO_BODY(cfa)));
    } else if (type == DEFINITION_TYPE_CALL || type == DEFINITION_TYPE_DOES) {
        // Colon definitions are executed by moving the program counter to the parameter field
        push(state->DEBUG_CALL_STACK, cfa);
        push(state->RETURN_STACK, state->program_counter);
        state->program_counter = *memory_at16(state->MEMORY, TO_CODE_P(cfa));

        if (type == DEFINITION_TYPE_DOES) {
            // If we are executing a DOES> word, we need to push the address of the word's parameter field to the stack
            push(state->DATA_STACK, TO_BODY(cfa));
        }
        
        // If we are not already executing a function, start executing it now
        if (state->RETURN_STACK->size == 1) {
            interpret_from_memory();
        }
        return 0;
    } else if (type == DEFINITION_TYPE_DEFERRED) {
        // Deferred words are executed by executing their parameter field
        return execute_word(*memory_at16(state->MEMORY, TO_BODY(cfa)));
    } else {
        // This should never happen
        return 1;
    }
}

void usage(FILE *file, char *program_name) {
    fprintf(file, "usage: %s [-M MEMORY_FILE] [-t]\n", program_name);
}

int main(int argc, char *argv[]) {
    char *memory_file = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(stdout, argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-M") == 0) {
            memory_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "-t") == 0) {
            TRACE = 1;
        } else {
            printf("error: unknown option %s\n", argv[i]);
            usage(stderr, argv[0]);
            return 1;
        }
    }
    init_forth();
    create_forth_vocabulary();
    execute_system();

    // Save memory to file
    if (memory_file != 0) {
        FILE *fp = fopen(memory_file, "wb");
        fwrite(state->MEMORY->memory, 1, MEMORY_SIZE, fp);
        fclose(fp);
    }

    printf("Virtual Forth 83\n");
    printf("Version 0.1\n");

    while (1) {
        int err;
        err = read_line_to_input_buffer(state);
        if (err == ERROR_END_OF_INPUT) break;
        else if (err != 0) {
            print_error(err);
            continue;
        }
        err = interpret_from_input_stream();
        if (err == 0) printf(" ok");
        printf("\n");
    }
    free_forth();
}