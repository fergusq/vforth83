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
#include "errors.h"

#define print_error(err) {\
        char *message = get_error_string(err);\
        printf("error: %s (%d)\n", message, err);\
    }

// *** Memory ***

InterpreterState *state;

// *** Initialization and cleanup ***

void init_forth() {
    state = malloc(sizeof(*state));
    state->RETURN_STACK = create_stack();
    state->DATA_STACK = create_stack();
    state->MEMORY = create_memory();
    state->program_counter = 0;
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
    state->BUILTINS[word] = addr - PARAMETER_P_OFFSET + TYPE_OFFSET;
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
    uint16_t FORTH_param_field = add_definition(state->MEMORY, "FORTH", 0, DEFINITION_TYPE_DOES, 0);
    insert16(state->MEMORY, FORTH_param_field - PARAMETER_P_OFFSET); // Add the word FORTH as the root of the FORTH chain
    //*memory_at16(state->MEMORY, FORTH_param_field - PARAMETER_P_OFFSET + CODE_P_OFFSET) = 0;
    *state->MEMORY->CURRENT_var = FORTH_param_field;

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
    state->VOC_LINK_var = memory_at16(state->MEMORY, add_variable("VOC-LINK"));
    add_constant("#VOCS", NUM_VOCS);
    state->FILE_var = memory_at16(state->MEMORY, add_variable("FILE"));

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

// *** Interpreter ***

// Forth has two interpreters: one that interprets words from the input stream, another that interprets compiled words from the memory

int interpret_from_input_stream() {
    int error = 0;
    while (1) {
        uint8_t *word = read_word(state);
        if (word == 0) {
            break;
        }
        if (strspn(word, "0123456789") == strlen(word)) {
            uint16_t num = atoi(word);
            if (*state->STATE_var != 0) {
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
                error = 1;
                break;
            }
            if (*state->STATE_var != 0 && !definition->is_immediate) {
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
                    char *message = get_error_string(ret);
                    printf("error: %s (%d) while executing word %s\n", message, ret, word);
                    free(word);
                    error = 1;
                    break;
                }
                free(word);
            }
        }
    }
    return error;
}

void interpret_from_memory() {
    while (1) {
        uint16_t instruction = *memory_at16(state->MEMORY, state->program_counter);
        //Definition *definition = get_definition(state->MEMORY, instruction - TYPE_OFFSET);
        //printf("Executing %d %s at %d\n", instruction, definition->name, state->program_counter);
        //free_definition(definition);
        state->program_counter += 2;
        if (instruction == state->BUILTINS[BUILTIN_WORD_LIT]) {
            uint16_t num = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += 2;
            push(state->DATA_STACK, num);
        } else if (instruction == state->BUILTINS[BUILTIN_WORD_BRANCH]) {
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += addr;
        } else if (instruction == state->BUILTINS[BUILTIN_WORD_QUESTION_BRANCH]) {
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
            int ret = execute_word(instruction);
            if (ret == -1) {
                break;
            } else if (ret > 0) {
                Definition *definition = get_definition(state->MEMORY, instruction - TYPE_OFFSET);
                char *message = get_error_string(ret);
                printf("error: %s (%d) while executing word %s\n", message, ret, definition->name);
                free_definition(definition);
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
        state->program_counter = p - TYPE_OFFSET + PARAMETER_P_OFFSET;
        
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

void usage(FILE *file, char *program_name) {
    fprintf(file, "usage: %s [-M MEMORY_FILE]\n", program_name);
}

int main(int argc, char *argv[]) {
    char *memory_file = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help")) {
            usage(stdout, argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-M") == 0) {
            memory_file = argv[i+1];
            i += 1;
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
        if (err != 0) printf(" ok");
        printf("\n");
    }
    free_forth();
}