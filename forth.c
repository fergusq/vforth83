#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

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

uint8_t TRACE = 0, VERBOSE = 0;

// *** Memory ***

InterpreterState *state;

// *** Initialization and cleanup ***

void init_forth() {
    state = malloc(sizeof(*state));
    state->DEBUG_CALL_STACK = create_stack();
    state->MEMORY = create_memory();
    state->program_counter = 0;
    state->breakpoint = 0;
    state->debug = 0;

    allot(state->MEMORY, 2*1024);
}

void free_forth() {
    free_stack(state->DEBUG_CALL_STACK);
    free_memory(state->MEMORY);
    free(state);
}

void add_builtin(char *name, enum BuiltinWord word, uint8_t is_immediate) {
    char *uppername = upper(name);
    if (BUILTINS[word] == 0) {
        return;
    }
    add_definition(state->MEMORY, uppername, is_immediate, DEFINITION_TYPE_BUILTIN, 1);
    uint16_t addr = insert16(state->MEMORY, word);
    state->BUILTINS[word] = FROM_BODY(addr);
    if (addr == 0) {
        printf("error while defining builtin %s\n", uppername);
    }
    free(uppername);
}

uint16_t add_variable(char *name) {
    char *uppername = upper(name);
    add_definition(state->MEMORY, uppername, 0, DEFINITION_TYPE_VARIABLE, 1);
    free(uppername);
    return allot(state->MEMORY, 2);
}

uint16_t add_constant(char *name, uint16_t value) {
    char *uppername = upper(name);
    add_definition(state->MEMORY, uppername, 0, DEFINITION_TYPE_CONSTANT, 1);
    free(uppername);
    return insert16(state->MEMORY, value);
}

void create_forth_vocabulary() {

    // Bootstrap vocabulary-related variables and reserve the FORTH vocabulary

    state->MEMORY->LAST_var = state->LAST_var = memory_at16(state->MEMORY, allot(state->MEMORY, 2));
    state->MEMORY->CURRENT_var = state->CURRENT_var = memory_at16(state->MEMORY, allot(state->MEMORY, 2)); // Temporary space for CURRENT
    uint16_t FORTH_pfa = add_definition(state->MEMORY, "FORTH", 0, DEFINITION_TYPE_DOES, 0);
    insert16(state->MEMORY, TO_NAME(state->MEMORY, FROM_BODY(FORTH_pfa))); // Add the word FORTH as the root of the FORTH chain
    //*memory_at16(state->MEMORY, FORTH_pfa - PARAMETER_P_OFFSET + CODE_P_OFFSET) = 0;
    *state->MEMORY->CURRENT_var = FORTH_pfa;

    // Add builtins

    add_builtins(add_builtin);

    // Create variables

    state->MEMORY->SP0_var = state->SP0_var = memory_at16(state->MEMORY, add_variable("SP0"));
    state->MEMORY->RP0_var = state->RP0_var = memory_at16(state->MEMORY, add_variable("RP0"));
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

    // Initialize variables

    *state->BASE_var = 10;
    *state->SP0_var = 61396 - 200;
    *state->RP0_var = 61396;

    // Fix CURRENT

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

void execute_system(char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == 0) {
        printf("error: could not open %s\n", filename);
        return;
    }

    // Read line by line and interpret
    while (1) {
        int err = read_paragraph_to_input_buffer_from_file(state, file);
        if (err == ERROR_END_OF_INPUT) break;
        else if (err != 0) {
            print_error(err);
            break;
        }
        if (VERBOSE) {
            for (int i = 0; i < *state->NUMBER_TIB_var; i++) {
                printf("%c", state->INPUT_BUFFER[i]);
            }
            printf("\n");
        }
        if (interpret_from_input_stream() != 0) break;
    }

    fclose(file);
}

// *** Debug ***

void print_stack_trace(FILE *file) {
    for (int i = state->DEBUG_CALL_STACK->size - 2; i >= 0; i-=2) {
        uint16_t cfa = state->DEBUG_CALL_STACK->bottom[i];
        Definition *dbg_call_stack_definition = get_definition(state->MEMORY, TO_NAME(state->MEMORY, cfa));
        fprintf(file, " < %s", dbg_call_stack_definition->name);
        free_definition(dbg_call_stack_definition);
    }
    fprintf(file, " || ");
    for (int i = state->MEMORY->return_stack_size - 1; i >= 0; i--) {
        uint16_t cfa;
        pick_return_stack(state->MEMORY, i, &cfa);
        fprintf(file, " < %d", cfa);
    }
}

void push_debug_frame(uint16_t cfa) {
    push(state->DEBUG_CALL_STACK, cfa);
    push(state->DEBUG_CALL_STACK, state->MEMORY->return_stack_size);
}

int pop_debug_frame(uint16_t *cfa) {
    // Will return -1 if return stack was higher, or -2 if it was lower than expected
    uint16_t size;
    if (pop(state->DEBUG_CALL_STACK, &size) == -1) return ERROR_DEBUG_STACK_UNDERFLOW;
    if (pop(state->DEBUG_CALL_STACK, cfa) == -1) return ERROR_DEBUG_STACK_UNDERFLOW;
    Definition *def;
    if (TRACE) def = get_definition(state->MEMORY, TO_NAME(state->MEMORY, *cfa));
    if (size > state->MEMORY->return_stack_size) {
        if (TRACE) {
            fprintf(stderr, "warning: return stack mismatch: %d != %d, popping frame with %u %s\n", state->MEMORY->return_stack_size, size, *cfa, def->name);
            free_definition(def);
        }
        // Return stack has is smaller than expected, so we will pop debug stack as well to see if we can make it match
        return pop_debug_frame(cfa);
    } else if (size < state->MEMORY->return_stack_size && TRACE) {
        fprintf(stderr, "warning: return stack mismatch: %d != %d\n", state->MEMORY->return_stack_size, size);
    }
    if (TRACE) {
        fprintf(stderr, "Leaving %d %s\n", *cfa, def->name);
        free_definition(def);
    }
    return 0;
}

void step(uint16_t next_cfa) {
    printf("Stack trace: ");
    print_stack_trace(stdout);
    printf("\n");
    Definition *def = get_definition(state->MEMORY, TO_NAME(state->MEMORY, next_cfa));
    printf("Stack:");
    for (int i = state->MEMORY->data_stack_size - 1; i >= 0; i--) {
        uint16_t value;
        pick_data_stack(state->MEMORY, i, &value);
        printf(" %7d", value);
    }
    printf("\n");
    printf("Next instruction: %s\n", def->name);
    loop: while (1) {
        printf("N=Next C=Continue > ");
        char cmd[10];
        fgets(cmd, 9, stdin);
        switch (cmd[0]) {
            case 'N':
            case 'n':
                goto end;
            case 'C':
            case 'c':
                state->debug = 0;
                state->breakpoint = 0;
                goto end;
            default:
                break;
        }
    }
    end:
}

// *** Interpreter ***

// Forth has two interpreters: one that interprets words from the input stream, another that interprets compiled words from the memory

int interpret_from_input_stream() {
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
        if (TRACE) fprintf(stderr, "Executing %s from input stream (state %d, >in %d)\n", word, *state->STATE_var, *state->TO_IN_var);
        Definition *definition = find_word(state->MEMORY, word);
        if (definition == 0) {
            // Try parsing it as a number
            int8_t sign = 1, index = 0, num_parsing_error = 0, len = strlen(word);
            uint16_t value = 0;
            if (word[0] == '-') { sign = -1; index += 1; }
            for (; index < len; index++) {
                uint8_t chr = word[index];
                if (chr > '9' && *state->BASE_var > 10) {
                    chr = toupperf(chr) - 'A' + 10;
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
                printf("%s?\n", word);
                free(word);
                return 1;
            } else {
                if (sign == -1) value = (1 << 16) - value;
                if (*state->STATE_var != 0) {
                    insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
                    insert16(state->MEMORY, value);
                } else {
                    int ret = push_data_stack(state->MEMORY, value);
                    if (ret != 0) return ret;
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
            //if (TRACE) fprintf(stderr, "Executing %d\n", FROM_BODY(definition->pfa));
            int ret = execute_word(FROM_BODY(definition->pfa));
            free_definition(definition);
            if (ret == -3) {
                free(word);
                return 1;
            } else if (ret == -2) {
                free(word);
                return 0;
            } else if (ret > 0) {
                char *message = get_error_string(ret);
                fprintf(stderr, "error: %s (%d) while executing word `%s'\n", message, ret, word);
                free(word);

                // reset state
                state->MEMORY->return_stack_size = 0;
                state->DEBUG_CALL_STACK->top = state->DEBUG_CALL_STACK->bottom; 
                state->DEBUG_CALL_STACK->size = 0;
                *state->STATE_var = 0;
                return 1;
            }
            free(word);
        }
    }
    return 0;
}

int interpret_from_memory() {
    while (1) {
        uint16_t cfa = *memory_at16(state->MEMORY, state->program_counter);
        state->program_counter += 2;
        if (state->breakpoint != 0 && state->breakpoint == cfa) {
            state->debug = 1;
        }
        if (state->debug) {
            step(cfa);
        }
        if (cfa == state->BUILTINS[BUILTIN_WORD_LIT]) {
            uint16_t num = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += 2;
            int ret = push_data_stack(state->MEMORY, num);
            if (ret != 0) return ret;
        } else if (cfa == state->BUILTINS[BUILTIN_WORD_BRANCH]) {
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            state->program_counter += addr;
        } else if (cfa == state->BUILTINS[BUILTIN_WORD_QUESTION_BRANCH]) {
            uint16_t flag;
            int ret = pop_data_stack(state->MEMORY, &flag);
            if (ret != 0) return ret;
            uint16_t addr = *memory_at16(state->MEMORY, state->program_counter);
            if (flag == 0) {
                state->program_counter += addr;
            } else {
                state->program_counter += 2;
            }
        } else {
            int ret = execute_word(cfa);
            if (ret < 0) {
                return ret;
            } else if (ret > 0) {
                Definition *definition = get_definition(state->MEMORY, TO_NAME(state->MEMORY, cfa));
                char *message = get_error_string(ret);
                fprintf(stderr, "error: %s (%d) while executing word %u %s", message, ret, cfa, definition->name);
                print_stack_trace(stderr);
                fprintf(stderr, "\n");
                free_definition(definition);
                return -3;
            }
        }
    }
    return 0;
}

// *** Execution ***

int execute_word(uint16_t cfa) {
    if (TRACE) {
        Definition *definition = get_definition(state->MEMORY, TO_NAME(state->MEMORY, cfa));
        fprintf(stderr, "Executing %d %s at %d", cfa, definition->name, state->program_counter);
        print_stack_trace(stderr);
        fprintf(stderr, "\n");
        free_definition(definition);
    }
    enum DefinitionType type = *memory_at16(state->MEMORY, cfa);
    if (type == DEFINITION_TYPE_VARIABLE) {
        // Variables push their parameter field address to the stack
        int ret = push_data_stack(state->MEMORY, TO_BODY(cfa));
        if (ret != 0) return ret;
        return 0;
    } else if (type == DEFINITION_TYPE_CONSTANT) {
        // Constants push their parameter field value to the stack
        int ret = push_data_stack(state->MEMORY, *memory_at16(state->MEMORY, TO_BODY(cfa)));
        if (ret != 0) return ret;
        return 0;
    } else if (type == DEFINITION_TYPE_BUILTIN) {
        // Builtins are executed with their respective functions in builtins.c
        int ret = execute_builtin(state, *memory_at16(state->MEMORY, TO_BODY(cfa)));
        return ret;
    } else if (type == DEFINITION_TYPE_CALL || type == DEFINITION_TYPE_DOES) {
        // Colon definitions are executed by moving the program counter to the parameter field
        push_debug_frame(cfa);
        int ret = push_return_stack(state->MEMORY, state->program_counter);
        if (ret != 0) return ret;
        state->program_counter = *memory_at16(state->MEMORY, TO_CODE_P(cfa));

        if (type == DEFINITION_TYPE_DOES) {
            // If we are executing a DOES> word, we need to push the address of the word's parameter field to the stack
            int ret = push_data_stack(state->MEMORY, TO_BODY(cfa));
            if (ret != 0) return ret;
        }
        
        // If we are not already executing a function, start executing it now
        if (state->MEMORY->return_stack_size == 1) {
            return interpret_from_memory();
        }
        return 0;
    } else if (type == DEFINITION_TYPE_DEFERRED) {
        // Deferred words are executed by executing their parameter field
        return execute_word(*memory_at16(state->MEMORY, TO_BODY(cfa)));
    } else {
        fprintf(stderr, "Unknown definition: %u\n", type);
        // This should never happen
        return ERROR_UNKNOWN_DEFINITION_TYPE;
    }
}

void usage(FILE *file, char *program_name) {
    fprintf(file, "usage: %s [-s SYSTEM_FILE] [-M MEMORY_FILE] [-t] [--finnish]\n", program_name);
}

int main(int argc, char *argv[]) {
    setlocale(LC_CTYPE, "");
    char *memory_file = 0;
    char *system_file = "system.f";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(stdout, argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-s") == 0) {
            system_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "-M") == 0) {
            memory_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "-t") == 0) {
            TRACE = 1;
        } else if (strcmp(argv[i], "--finnish") == 0) {
            FINNISH = 1;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            VERBOSE = 1;
        } else {
            fprintf(stderr, "error: unknown option %s\n", argv[i]);
            usage(stderr, argv[0]);
            return 1;
        }
    }
    init_forth();
    create_forth_vocabulary();
    execute_system(system_file);

    // Save memory to file
    if (memory_file != 0) {
        FILE *fp = fopen(memory_file, "wb");
        fwrite(state->MEMORY->memory, 1, MEMORY_SIZE, fp);
        fclose(fp);
    }

    printf("---\n");

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