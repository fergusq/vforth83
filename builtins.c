#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include "builtins.h"
#include "stack.h"
#include "input_stream.h"
#include "dos.h"
#include "memory.h"
#include "forth.h"
#include "errors.h"
#include "util.h"
#include "io.h"

#define modulo(i, n) (((i) % (n) + n) % (n))

#define pop_to_unsigned(varname) uint16_t varname;\
    {\
        int ret = pop_data_stack(state->MEMORY, &varname);\
        if (ret != 0) {\
            return ret;\
        }\
    }

#define pop_to_signed(varname) int16_t varname;\
    {\
        int ret = pop_data_stack(state->MEMORY, &varname);\
        if (ret != 0) {\
            return ret;\
        }\
    }

#define pop_return_to_unsigned(varname) uint16_t varname;\
    {\
        int ret = pop_return_stack(state->MEMORY, &varname);\
        if (ret != 0) {\
            return ret;\
        }\
    }

#define pop_to_unsigned_32bit(varname) pop_to_unsigned(varname ## _a);\
    pop_to_unsigned(varname ## _b);\
    uint32_t varname = ((uint32_t) varname ## _a << 16) + varname ## _b;

#define pop_to_signed_32bit(varname) pop_to_unsigned(varname ## _a);\
    pop_to_unsigned(varname ## _b);\
    int32_t varname = (int32_t) ((uint32_t) varname ## _a << 16) + varname ## _b;

#define push_from(varname) {\
    int ret = push_data_stack(state->MEMORY, varname);\
    if (ret != 0) return ret;\
}

#define push_return_from(varname) {\
    int ret = push_return_stack(state->MEMORY, varname);\
    if (ret != 0) return ret;\
}

#define push_from_32bit(varname) push_from(((uint32_t) varname) & 0xFFFF);\
    push_from(((uint32_t) varname) >> 16);

#define read_next_word(word) char *word = read_word(state);\
    if (word == 0) {\
        return ERROR_END_OF_INPUT;\
    }\
    if (*state->CAPS_var != 0) {\
        for (uint8_t i = 0; i < strlen(word); i++) {\
            word[i] = toupperf(word[i]);\
        }\
    }

#define read_name(word, definition) read_next_word(word)\
    Definition *definition = find_word(state->MEMORY, word);\
    if (definition == 0) {\
        forth_printf("%s?\n", word);\
        free(word);\
        return ERROR_WORD_NOT_FOUND;\
    }

// Forward declarations

int builtin_quit(InterpreterState *state);
int builtin_exit(InterpreterState *state);

// Builtin definitions

int builtin_store(InterpreterState *state) {
    pop_to_unsigned(address);
    pop_to_unsigned(value);
    *memory_at16(state->MEMORY, address) = value;
    return 0;
}

// #
// #>
// #S

int builtin_tick(InterpreterState *state) {
    read_name(word, definition);
    push_from(FROM_BODY(definition->pfa));
    free_definition(definition);
    free(word);
    return 0;
}

int builtin_paren(InterpreterState *state) {
    int c;
    while ((c = read_char(state)) != ')') {
        if (c == 0) {
            return ERROR_END_OF_INPUT;
        }
    }
    return 0;
}

int builtin_times(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    int16_t product = n1 * n2;
    push_from(product);
    return 0;
}

int builtin_times_divide(InterpreterState *state) {
    pop_to_signed(n3);
    pop_to_signed(n2);
    pop_to_signed(n1);
    if (n3 == 0) return ERROR_DIVISION_BY_ZERO;
    int32_t product = n1 * n2;
    int16_t quotient = product / n3;
    if ((product < 0 && n3 > 0 || product > 0 && n3 < 0) && product%n3 != 0) quotient -= 1; // This is floored division
    push_from(quotient);
    return 0;
}

int builtin_times_divide_mod(InterpreterState *state) {
    pop_to_signed(n3);
    pop_to_signed(n2);
    pop_to_signed(n1);
    if (n3 == 0) return ERROR_DIVISION_BY_ZERO;
    int32_t product = n1 * n2;
    int16_t quotient = product / n3;
    int16_t remainder = modulo(product, n3);
    if ((product < 0 && n3 > 0 || product > 0 && n3 < 0) && remainder != 0) quotient -= 1; // This is floored division
    push_from(remainder);
    push_from(quotient);
    return 0;
}

int builtin_plus(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 + n2);
    return 0;
}

// +LOOP

int builtin_plus_store(InterpreterState *state) {
    pop_to_unsigned(address);
    pop_to_signed(value);
    *memory_at16(state->MEMORY, address) += value;
    return 0;
}

int builtin_comma(InterpreterState *state) {
    pop_to_unsigned(value);
    insert16(state->MEMORY, value);
    return 0;
}

int builtin_minus(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 - n2);
    return 0;
}

int builtin_dash_trailing(InterpreterState *state) {
    pop_to_unsigned(len);
    pop_to_unsigned(address);
    do {
        uint8_t c = *memory_at8(state->MEMORY, address+len-1);
        if (!isspace(c)) break;
        len -= 1;
    } while (len > 0);
    push_from(address);
    push_from(len);
    return 0;
}

int builtin_dot(InterpreterState *state) {
    pop_to_signed(n);
    forth_printf("%d ", n);
    return 0;
}

int builtin_dot_quote(InterpreterState *state) {
    int c;
    int flag = 1;
    while ((c = read_char(state)) != '"') {
        if (flag) {
            if (isspace(c)) {
                c = read_char(state);
            }
        }
        flag = 0;
        if (c == '"') break;
        if (c == 0) {
            return ERROR_END_OF_INPUT;
        }
        insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
        insert16(state->MEMORY, c);
        insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_EMIT]);
    }
    return 0;
}

int builtin_dot_paren(InterpreterState *state) {
    int c;
    int flag = 1;
    while ((c = read_char(state)) != ')') {
       if (flag) {
            if (isspace(c)) {
                c = read_char(state);
            }
        }
        flag = 0;
        if (c == ')') break;
        if (c == 0) {
            return ERROR_END_OF_INPUT;
        }
        forth_addch(c);
    }
    return 0;
}

int builtin_divide(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    //if (n2 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n2 == 0) {
        push_from(0);
        return 0;
    }
    int16_t quotient = n1 / n2;
    if ((n1 < 0 && n2 > 0 || n1 > 0 && n2 < 0) && n1%n2 != 0) quotient -= 1; // This is floored division
    push_from(quotient);
    return 0;
}

int builtin_divide_mod(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    //if (n2 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n2 == 0) {
        push_from(1);
        push_from(0);
        return 0;
    }
    int16_t quotient = n1 / n2;
    int16_t remainder = modulo(n1, n2);
    if ((n1 < 0 && n2 > 0 || n1 > 0 && n2 < 0) && remainder != 0) quotient -= 1; // This is floored division
    push_from(remainder);
    push_from(quotient);
    return 0;
}

int builtin_zero_less(InterpreterState *state) {
    pop_to_signed(n);
    push_from(n < 0 ? -1 : 0);
    return 0;
}

int builtin_zero_equals(InterpreterState *state) {
    pop_to_signed(n);
    push_from(n == 0 ? -1 : 0);
    return 0;
}

int builtin_zero_greater(InterpreterState *state) {
    pop_to_signed(n);
    push_from(n > 0 ? -1 : 0);
    return 0;
}

int builtin_one_plus(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(n + 1);
    return 0;
}

int builtin_one_minus(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(n - 1);
    return 0;
}

int builtin_two_plus(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(n + 2);
    return 0;
}

int builtin_two_minus(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(n - 2);
    return 0;
}

int builtin_two_divide(InterpreterState *state) {
    pop_to_signed(n);
    push_from(n >> 1);
    return 0;
}

int builtin_colon(InterpreterState *state) {
    read_next_word(name);
    uint16_t pfa = add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_CALL, 0);
    *memory_at16(state->MEMORY, TO_CODE_P(FROM_BODY(pfa))) = FROM_BODY(pfa);
    *state->CONTEXT_var = *state->CURRENT_var;
    *state->STATE_var = 1;
    push_from(TO_NAME(state->MEMORY, FROM_BODY(pfa)));
    free(name);
    return 0;
}

int builtin_semi_colon(InterpreterState *state) {
    pop_to_unsigned(addr); // sys
    *state->STATE_var = 0;
    *memory_at16(state->MEMORY, *state->CURRENT_var) = addr;
    insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_EXIT]);
    return 0;
}

int builtin_less_than(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 < n2 ? -1 : 0);
    return 0;
}

// <#

int builtin_equals(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 == n2 ? -1 : 0);
    return 0;
}

int builtin_greater_than(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 > n2 ? -1 : 0);
    return 0;
}

// >BODY

int builtin_to_r(InterpreterState *state) {
    pop_to_unsigned(value);
    push_return_from(value);
    return 0;
}

int builtin_question_dup(InterpreterState *state) {
    pop_to_unsigned(value);
    push_from(value);
    if (value != 0) {
        push_from(value);
    }
    return 0;
}

int builtin_fetch(InterpreterState *state) {
    pop_to_unsigned(addr);
    push_from(*memory_at16(state->MEMORY, addr));
    return 0;
}

int builtin_abort(InterpreterState *state) {
    state->MEMORY->data_stack_size = 0;
    print_stack_trace(stdout);
    return builtin_quit(state);
}

// ABORT"

int builtin_abs(InterpreterState *state) {
    pop_to_signed(n);
    push_from(n < 0 ? -n : n);
    return 0;
}

int builtin_allot(InterpreterState *state) {
    pop_to_signed(w);
    allot(state->MEMORY, w);
    return 0;
}

int builtin_and(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 & n2);
    return 0;
}

// BEGIN
// BLOCK
// BUFFER

int builtin_c_store(InterpreterState *state) {
    pop_to_unsigned(addr);
    pop_to_unsigned(value);
    *memory_at8(state->MEMORY, addr) = value;
    return 0;
}

int builtin_c_fetch(InterpreterState *state) {
    pop_to_unsigned(addr);
    push_from(*memory_at8(state->MEMORY, addr));
    return 0;
}

int builtin_cmove(InterpreterState *state) {
    pop_to_unsigned(count);
    pop_to_unsigned(to);
    pop_to_unsigned(from);
    for (uint16_t i = 0; i < count; i++) {
        *memory_at8(state->MEMORY, to + i) = *memory_at8(state->MEMORY, from + i);
    }
    return 0;
}

int builtin_cmove_up(InterpreterState *state) {
    pop_to_unsigned(count);
    pop_to_unsigned(to);
    pop_to_unsigned(from);
    for (int32_t i = count - 1; i >= 0; i--) {
        *memory_at8(state->MEMORY, to + i) = *memory_at8(state->MEMORY, from + i);
    }
    return 0;
}

int builtin_compile(InterpreterState *state) {
    // : COMPILE R> DUP 2+ >R @ , ;
    /*uint16_t return_addr;
    if (pop(state->RETURN_STACK, &return_addr) == -1) {
        return ERROR_RETURN_STACK_UNDERFLOW;
    }
    push(state->RETURN_STACK, return_addr + 2);
    insert16(state->MEMORY, *memory_at16(state->MEMORY, return_addr));
    return 0;*/
}

int builtin_constant(InterpreterState *state) {
    pop_to_unsigned(value);
    read_next_word(name);
    add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_CONSTANT, 1);
    insert16(state->MEMORY, value);
    free(name);
    return 0;
}

// CONVERT

int builtin_count(InterpreterState *state) {
    pop_to_unsigned(addr);
    uint8_t len = *memory_at8(state->MEMORY, addr);
    push_from(addr + 1);
    push_from(len);
    return 0;
}

int builtin_cr(InterpreterState *state) {
    forth_printf("\n");
    return 0;
}

int builtin_create(InterpreterState *state) {
    read_next_word(name);
    add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_VARIABLE, 1);
    free(name);
    return 0;
}

int builtin_d_plus(InterpreterState *state) {
    pop_to_unsigned_32bit(d2);
    pop_to_unsigned_32bit(d1);
    uint32_t sum = d1 + d2;
    push_from_32bit(sum);
    return 0;
}

int builtin_d_less_than(InterpreterState *state) {
    pop_to_signed_32bit(d2);
    pop_to_signed_32bit(d1);
    push_from(d1 < d2 ? -1 : 0);
    return 0;
}

// DECIMAL
// DEFINITIONS

int builtin_depth(InterpreterState *state) {
    push_from(state->MEMORY->data_stack_size);
    return 0;
}

int builtin_dnegate(InterpreterState *state) {
    pop_to_signed_32bit(d1);
    push_from_32bit(-d1);
    return 0;
}

// DO

int builtin_does(InterpreterState *state) {
    uint16_t pc = state->program_counter;
    uint16_t nfa = *state->MEMORY->LAST_var;
    *memory_at16(state->MEMORY, FROM_NAME(state->MEMORY, nfa)) = DEFINITION_TYPE_DOES;
    *memory_at16(state->MEMORY, TO_CODE_P(FROM_NAME(state->MEMORY, nfa))) = pc;
    return builtin_exit(state);
}

int builtin_drop(InterpreterState *state) {
    pop_to_unsigned(value);
    return 0;
}

int builtin_dup(InterpreterState *state) {
    pop_to_unsigned(value);
    push_from(value);
    push_from(value);
    return 0;
}

// ELSE

int builtin_emit(InterpreterState *state) {
    pop_to_unsigned(value);
    forth_addch(value);
    return 0;
}

int builtin_execute(InterpreterState *state) {
    pop_to_unsigned(p);
    return execute_word(p);
}

int builtin_exit(InterpreterState *state) {
    uint16_t debug_addr;
    pop_return_to_unsigned(ret_addr);
    pop_debug_frame(&debug_addr);
    state->program_counter = ret_addr;
    if (ret_addr == 0) {
        return -1;
    }
    return 0;
}

int builtin_expect(InterpreterState *state) {
    pop_to_unsigned(max_count);
    pop_to_unsigned(addr);
    uint16_t count = 0;
    int c;
    while ((c = getchar()) != ' ') {
        if (c == 0 || c == '\n') {
            return 0;
        }
        if (count >= max_count) {
            return 0;
        }
        *memory_at8(state->MEMORY, addr + count) = c;
        count += 1;
    }
}

int builtin_fill(InterpreterState *state) {
    pop_to_unsigned(value);
    pop_to_unsigned(count);
    pop_to_unsigned(addr);
    for (uint16_t i = 0; i < count; i++) {
        *memory_at8(state->MEMORY, addr + i) = value;
    }
    return 0;
}

int builtin_find(InterpreterState *state) {
    pop_to_unsigned(addr); // Counted string
    uint8_t len = *memory_at8(state->MEMORY, addr);
    uint8_t *name = malloc(len + 1);
    for (uint8_t i = 0; i < len; i++) {
        name[i] = *memory_at8(state->MEMORY, addr + i + 1);
    }
    name[len] = 0;
    Definition *definition = find_word(state->MEMORY, name);
    free(name);
    if (definition == 0) {
        push_from(addr);
        push_from(0);
    } else {
        push_from(FROM_BODY(definition->pfa));
        if (definition->is_immediate) {
            push_from(1);
        } else {
            push_from(-1);
        }
        free_definition(definition);
    }
    return 0;
}

// FLUSH

int builtin_forget(InterpreterState *state) {
    read_name(word, definition);
    forget(state->MEMORY, definition);
    free_definition(definition);
    free(word);
    return 0;
}

// FORTH

int builtin_forth_83(InterpreterState *state) {
    return 0;
}

int builtin_here(InterpreterState *state) {
    push_from(state->MEMORY->memory_pointer);
    return 0;
}

// HOLD
// I
// IF

int builtin_immediate(InterpreterState *state) {
    uint16_t nfa = *state->MEMORY->LAST_var;
    *memory_at8(state->MEMORY, TO_IMMEDIATE_FLAG(FROM_NAME(state->MEMORY, nfa))) = 1;
    return 0;
}

// J

int builtin_key(InterpreterState *state) {
    int c = getchar();
    if (c == 0) {
        return ERROR_END_OF_INPUT;
    }
    push_from(c);
    return 0;
}

// LEAVE

int builtin_literal(InterpreterState *state) {
    pop_to_unsigned(value);
    insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
    insert16(state->MEMORY, value);
    return 0;
}

// LOAD
// LOOP

int builtin_max(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 > n2 ? n1 : n2);
    return 0;
}

int builtin_min(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(n1 < n2 ? n1 : n2);
    return 0;
}

int builtin_mod(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    push_from(modulo(n1, n2));
    return 0;
}

int builtin_negate(InterpreterState *state) {
    pop_to_signed(n);
    push_from(-n);
    return 0;
}

int builtin_not(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(~n);
    return 0;
}

int builtin_or(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 | n2);
    return 0;
}

int builtin_over(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1);
    push_from(n2);
    push_from(n1);
    return 0;
}

// PAD

int builtin_pick(InterpreterState *state) {
    pop_to_unsigned(n);
    uint16_t value;
    int ret = pick_data_stack(state->MEMORY, n, &value);
    if (ret != 0) return ret;
    push_from(value);
    return 0;
}

int builtin_quit(InterpreterState *state) {
    state->MEMORY->return_stack_size = 0;
    state->DEBUG_CALL_STACK->top = state->DEBUG_CALL_STACK->bottom; 
    state->DEBUG_CALL_STACK->size = 0;
    *state->STATE_var = 0;
    state->program_counter = 0;
    return -2;
}

int builtin_r_from(InterpreterState *state) {
    pop_return_to_unsigned(value);
    push_from(value);
    return 0;
}

int builtin_r_fetch(InterpreterState *state) {
    uint16_t value;
    int ret = pick_return_stack(state->MEMORY, 0, &value);
    if (ret != 0) return ret;
    push_from(value);
    return 0;
}

// REPEAT

int builtin_roll(InterpreterState *state) {
    pop_to_unsigned(n);
    if (state->MEMORY->data_stack_size <= n) {
        return ERROR_DATA_STACK_UNDERFLOW;
    }
    uint16_t sp = *state->MEMORY->SP0_var - 2*state->MEMORY->data_stack_size + 2;
    uint16_t value = *memory_at16(state->MEMORY, sp + 2*n);
    for (uint16_t i = n; i > 0; i--) {
        *memory_at16(state->MEMORY, sp + 2*i) = *memory_at16(state->MEMORY, sp + 2*i - 2);
    }
    *memory_at16(state->MEMORY, sp) = value;
    return 0;
}

int builtin_rot(InterpreterState *state) {
    pop_to_unsigned(n3);
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n2);
    push_from(n3);
    push_from(n1);
    return 0;
}

// SAVE-BUFFERS
// SIGN

int builtin_space(InterpreterState *state) {
    forth_addch(' ');
    return 0;
}

int builtin_spaces(InterpreterState *state) {
    pop_to_unsigned(n);
    for (uint16_t i = 0; i < n; i++) {
        forth_addch(' ');
    }
    return 0;
}

int builtin_swap(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n2);
    push_from(n1);
    return 0;
}

// THEN
// TIB

int builtin_type(InterpreterState *state) {
    pop_to_unsigned(count);
    pop_to_unsigned(addr);
    for (uint16_t i = 0; i < count; i++) {
        uint8_t value = *memory_at8(state->MEMORY, addr + i);
        forth_addch(value);
    }
    return 0;
}

int builtin_u_dot(InterpreterState *state) {
    pop_to_unsigned(n);
    forth_printf("%u ", n);
    return 0;
}

int builtin_u_less_than(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 < n2 ? -1 : 0);
    return 0;
}

int builtin_um_times(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    uint32_t product = ((uint32_t) n1) * n2;
    push_from_32bit(product);
    return 0;
}

int builtin_um_divide_mod(InterpreterState *state) {
    pop_to_unsigned(n3);
    pop_to_unsigned_32bit(d1);
    //if (n3 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n3 == 0) {
        push_from(-1);
        push_from(-1);
        return 0;
    }
    uint32_t quotient = d1 / n3;
    uint32_t remainder = modulo(d1, n3);
    push_from(remainder);
    push_from(quotient);
    return 0;
}

// UPDATE

int builtin_variable(InterpreterState *state) {
    read_next_word(name);
    add_definition(state->MEMORY, name, 0, DEFINITION_TYPE_VARIABLE, 1);
    allot(state->MEMORY, 2);
    free(name);
    return 0;
}

// VOCABULARY
// WHILE
// WORD

int builtin_xor(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 ^ n2);
    return 0;
}

int builtin_left_bracket(InterpreterState *state) {
    *state->STATE_var = 0;
    return 0;
}

int builtin_bracket_tick(InterpreterState *state) {
    read_name(word, definition);
    uint16_t addr = FROM_BODY(definition->pfa);
    free_definition(definition);
    free(word);

    insert16(state->MEMORY, state->BUILTINS[BUILTIN_WORD_LIT]);
    insert16(state->MEMORY, addr);

    return 0;
}

int builtin_bracket_compile(InterpreterState *state) {
    read_name(word, definition);
    uint16_t addr = FROM_BODY(definition->pfa);
    free_definition(definition);
    free(word);

    insert16(state->MEMORY, addr);

    return 0;
}

int builtin_right_bracket(InterpreterState *state) {
    *state->STATE_var = 1;
    return 0;
}

int builtin_two_store(InterpreterState *state) {
    pop_to_unsigned(addr);
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    *memory_at16(state->MEMORY, addr) = value2;
    *memory_at16(state->MEMORY, addr + 2) = value1;
    return 0;
}

int builtin_two_fetch(InterpreterState *state) {
    pop_to_unsigned(addr);
    push_from(*memory_at16(state->MEMORY, addr + 2));
    push_from(*memory_at16(state->MEMORY, addr));
    return 0;
}

int builtin_two_drop(InterpreterState *state) {
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    return 0;
}

int builtin_two_dup(InterpreterState *state) {
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value1);
    push_from(value2);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_two_over(InterpreterState *state) {
    pop_to_unsigned(value4);
    pop_to_unsigned(value3);
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value1);
    push_from(value2);
    push_from(value3);
    push_from(value4);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_two_rot(InterpreterState *state) {
    pop_to_unsigned(value6);
    pop_to_unsigned(value5);
    pop_to_unsigned(value4);
    pop_to_unsigned(value3);
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value3);
    push_from(value4);
    push_from(value5);
    push_from(value6);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_two_swap(InterpreterState *state) {
    pop_to_unsigned(value4);
    pop_to_unsigned(value3);
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value3);
    push_from(value4);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_d_minus(InterpreterState *state) {
    pop_to_unsigned_32bit(d2);
    pop_to_unsigned_32bit(d1);
    uint32_t difference = d1 - d2;
    push_from_32bit(difference);
    return 0;
}

int builtin_d_dot(InterpreterState *state) {
    pop_to_unsigned_32bit(d1);
    forth_printf("%d ", d1);
    return 0;
}

int builtin_d_dot_r(InterpreterState *state) {
    pop_to_unsigned(n);
    pop_to_unsigned_32bit(d1);
    char format[10];
    sprintf(format, "%%%dd ", n);
    forth_printf(format, d1);
    return 0;
}

int builtin_d_zero_equals(InterpreterState *state) {
    pop_to_unsigned_32bit(d1);
    push_from(d1 == 0 ? -1 : 0);
    return 0;
}

int builtin_d_two_divide(InterpreterState *state) {
    pop_to_signed_32bit(d1);
    push_from_32bit(d1 >> 1);
    return 0;
}

int builtin_d_equal(InterpreterState *state) {
    pop_to_unsigned_32bit(d2);
    pop_to_unsigned_32bit(d1);
    push_from(d1 == d2 ? -1 : 0);
    return 0;
}

int builtin_dabs(InterpreterState *state) {
    pop_to_signed_32bit(d1);
    uint32_t abs = (uint32_t) (d1 < 0 ? -d1 : d1);
    push_from_32bit(abs)
    return 0;
}

int builtin_dmax(InterpreterState *state) {
    pop_to_signed_32bit(d2);
    pop_to_signed_32bit(d1);
    push_from(d1 > d2 ? d1 : d2);
    return 0;
}

int builtin_dmin(InterpreterState *state) {
    pop_to_signed_32bit(d2);
    pop_to_signed_32bit(d1);
    push_from(d1 < d2 ? d1 : d2);
    return 0;
}

int builtin_du_less_than(InterpreterState *state) {
    pop_to_unsigned_32bit(d2);
    pop_to_unsigned_32bit(d1);
    push_from(d1 < d2 ? -1 : 0);
    return 0;
}

int builtin_backward_mark(InterpreterState *state) {
    push_from(state->MEMORY->memory_pointer);
    return 0;
}

int builtin_backward_resolve(InterpreterState *state) {
    pop_to_unsigned(addr);
    uint16_t offset = addr - state->MEMORY->memory_pointer;
    insert16(state->MEMORY, offset);
    return 0;
}

int builtin_forward_mark(InterpreterState *state) {
    push_from(state->MEMORY->memory_pointer);
    insert16(state->MEMORY, 0);
    return 0;
}

int builtin_forward_resolve(InterpreterState *state) {
    pop_to_unsigned(addr);
    uint16_t offset = state->MEMORY->memory_pointer - addr;
    *memory_at16(state->MEMORY, addr) = offset;
    return 0;
}

int builtin_dot_r(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_signed(n1);
    char format[10];
    sprintf(format, "%%%dd ", n2);
    forth_printf(format, n1);
    return 0;
}

int builtin_two_times(InterpreterState *state) {
    pop_to_unsigned(n);
    push_from(n << 1);
    return 0;
}

int builtin_interpret(InterpreterState *state) {
    return interpret_from_input_stream();
}

int builtin_u_dot_r(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    char format[10];
    sprintf(format, "%%%du ", n2);
    forth_printf(format, n1);
    return 0;
}

int builtin_not_equal(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 != n2 ? -1 : 0);
    return 0;
}

int builtin_include(InterpreterState *state) {
    char *word = read_word(state);
    if (word == 0) return ERROR_END_OF_INPUT;
    execute_system(word);
    return 0;
}

int builtin_dash_rot(InterpreterState *state) {
    pop_to_unsigned(value3);
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value3);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_tuck(InterpreterState *state) {
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value2);
    push_from(value1);
    push_from(value2);
    return 0;
}

int builtin_nip(InterpreterState *state) {
    pop_to_unsigned(value2);
    pop_to_unsigned(value1);
    push_from(value2);
    return 0;
}

int builtin_skip(InterpreterState *state) {
    pop_to_unsigned(chr);
    pop_to_unsigned(len);
    pop_to_unsigned(addr);
    while (len > 0) {
        if (*memory_at8(state->MEMORY, addr) != chr) {
            break;
        }
        addr += 1;
        len -= 1;
    }
    push_from(addr);
    push_from(len);
    return 0;
}

int builtin_scan(InterpreterState *state) {
    pop_to_unsigned(chr);
    pop_to_unsigned(len);
    pop_to_unsigned(addr);
    while (len > 0) {
        if (*memory_at8(state->MEMORY, addr) == chr) {
            break;
        }
        addr += 1;
        len -= 1;
    }
    push_from(addr);
    push_from(len);
    return 0;
}

int builtin_zero_not_equal(InterpreterState *state) {
    pop_to_unsigned(value);
    push_from(value != 0 ? -1 : 0);
    return 0;
}

int builtin_bdos(InterpreterState *state) {
    pop_to_unsigned(syscall);
    pop_to_unsigned(param);
    if (syscall > 255) {
        return ERROR_UNKNOWN_SYSCALL;
    }
    SysCallFunction syscall_func = DOS_SYSCALLS[syscall];
    if (syscall_func == 0) {
        return ERROR_UNKNOWN_SYSCALL;
    }
    uint8_t result = syscall_func(state->MEMORY, param);
    push_from(result);
    return 0;
}

int builtin_int10h(InterpreterState *state) {
    pop_to_unsigned(di);
    pop_to_unsigned(bp);
    pop_to_unsigned(dx);
    pop_to_unsigned(cx);
    pop_to_unsigned(bx);
    pop_to_unsigned(ax);
    forth_int10h(state->MEMORY, &ax, &bx, &cx, &dx, &bp, &di);
    push_from(ax);
    push_from(bx);
    push_from(cx);
    push_from(dx);
    push_from(bp);
    push_from(di);
    return 0;
}

int builtin_linemove(InterpreterState *state) {
    pop_to_unsigned(dest);
    pop_to_unsigned(source);
    forth_linemove(source, dest);
    return 0;
}

int builtin_upper(InterpreterState *state) {
    pop_to_unsigned(len);
    pop_to_unsigned(addr);
    for (uint8_t i = 0; i < len; i++) {
        uint8_t *c = memory_at8(state->MEMORY, addr + i);
        *c = toupperf(*c);
    }
    return 0;
}

int builtin_digit(InterpreterState *state) {
    pop_to_unsigned(base);
    pop_to_unsigned(chr);
    int upper_chr = toupperf(chr);
    if (chr >= '0' && chr <= '9' && chr-'0' < base) {
        push_from(chr - '0');
        push_from(-1);
    } else if (upper_chr >= 'A' && upper_chr-'A' < base-10) {
        push_from(upper_chr - 'A' + 10);
        push_from(-1);
    } else {
        push_from(chr);
        push_from(0);
    }
    return 0;
}

int builtin_rpick(InterpreterState *state) {
    pop_to_unsigned(n);
    uint16_t value;
    int ret = pick_return_stack(state->MEMORY, n, &value);
    if (ret != 0) return ret;
    push_from(value);
    return 0;
}

int builtin_sp_fetch(InterpreterState *state) {
    uint16_t sp = *state->MEMORY->SP0_var - 2*state->MEMORY->data_stack_size + 2;
    push_from(sp);
    return 0;
}

int builtin_rp_fetch(InterpreterState *state) {
    uint16_t rp = *state->MEMORY->RP0_var - 2*state->MEMORY->return_stack_size + 2;
    push_from(rp);
    return 0;
}

int builtin_sp_store(InterpreterState *state) {
    pop_to_unsigned(new_sp);
    uint16_t i = (*state->MEMORY->SP0_var - new_sp + 2);
    state->MEMORY->data_stack_size = i / 2;
    return 0;
}

int builtin_rp_store(InterpreterState *state) {
    pop_to_unsigned(new_rp);
    uint16_t i = (*state->MEMORY->RP0_var - new_rp + 2);
    state->MEMORY->return_stack_size = i / 2;
    return 0;
}

int builtin_u_two_divide(InterpreterState *state) {
    pop_to_unsigned(u);
    push_from(u >> 1);
    return 0;
}

int builtin_u_greater_than(InterpreterState *state) {
    pop_to_unsigned(n2);
    pop_to_unsigned(n1);
    push_from(n1 > n2 ? -1 : 0);
    return 0;
}

int builtin_s_to_d(InterpreterState *state) {
    pop_to_signed(n);
    int32_t n32 = n;
    push_from_32bit(n32);
    return 0;
}

int builtin_comp(InterpreterState *state) {
    pop_to_unsigned(len);
    pop_to_unsigned(addr2);
    pop_to_unsigned(addr1);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t a = *memory_at8(state->MEMORY, addr1+i);
        uint8_t b = *memory_at8(state->MEMORY, addr2+i);
        if (a < b) {
            push_from(-1);
            return 0;
        } else if (a > b) {
            push_from(1);
            return 0;
        }
    }
    push_from(0);
    return 0;
}

int builtin_caps_comp(InterpreterState *state) {
    pop_to_unsigned(len);
    pop_to_unsigned(addr2);
    pop_to_unsigned(addr1);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t a = *memory_at8(state->MEMORY, addr1+i);
        uint8_t b = *memory_at8(state->MEMORY, addr2+i);
        a = toupperf(a);
        b = toupperf(b);
        if (a < b) {
            push_from(-1);
            return 0;
        } else if (a > b) {
            push_from(1);
            return 0;
        }
    }
    push_from(0);
    return 0;
}

int builtin_lshift(InterpreterState *state) {
    pop_to_unsigned(u);
    pop_to_unsigned(n);
    push_from(n << u);
    return 0;
}

int builtin_rshift(InterpreterState *state) {
    pop_to_unsigned(u);
    pop_to_unsigned(n);
    push_from(n >> u);
    return 0;
}

int builtin_m_times(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed(n1);
    int32_t product = ((int32_t) n1) * n2;
    push_from_32bit(product);
    return 0;
}

int builtin_m_times_divide(InterpreterState *state) {
    pop_to_signed(n3);
    pop_to_signed(n2);
    pop_to_signed_32bit(d1);
    //if (n3 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n3 == 0) {
        push_from_32bit(-1);
        return 0;
    }
    int32_t product = d1 * n2;
    int32_t quotient = product / n3;
    push_from_32bit(quotient);
    return 0;
}

int builtin_fm_divide_mod(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed_32bit(d1);
    //if (n2 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n2 == 0) {
        push_from(-1);
        push_from(-1);
        return 0;
    }
    int16_t quotient = d1 / n2;
    int16_t remainder = modulo(d1, n2);
    if ((d1 < 0 && n2 > 0 || d1 > 0 && n2 < 0) && remainder != 0) quotient -= 1; // This is floored division
    push_from(remainder);
    push_from(quotient);
    return 0;
}

int builtin_sm_divide_rem(InterpreterState *state) {
    pop_to_signed(n2);
    pop_to_signed_32bit(d1);
    //if (n2 == 0) return ERROR_DIVISION_BY_ZERO;
    if (n2 == 0) {
        push_from(-1);
        push_from(-1);
        return 0;
    }
    int16_t quotient = d1 / n2;
    int16_t remainder = d1 % n2;
    push_from(remainder);
    push_from(quotient);
    return 0;
}

int builtin_finnish(InterpreterState *state) {
    pop_to_signed(u);
    FINNISH = u != 0;
    return 0;
}

void see(InterpreterState *state, Definition *definition) {
    if (definition->type == DEFINITION_TYPE_VARIABLE) {
        forth_printf("VARIABLE %s Value = %d", definition->name, *memory_at16(state->MEMORY, definition->pfa));
    } else if (definition->type == DEFINITION_TYPE_CONSTANT) {
        uint16_t value = *memory_at16(state->MEMORY, definition->pfa);
        forth_printf("%d CONSTANT %s", value, definition->name);
    } else if (definition->type == DEFINITION_TYPE_BUILTIN) {
        forth_printf("BUILTIN %s", definition->name);
        if (definition->is_immediate) {
            forth_printf(" IMMEDIATE");
        }
    } else if (definition->type == DEFINITION_TYPE_CALL || definition->type == DEFINITION_TYPE_DOES) {
        uint16_t code_p = definition->code_p;
        if (definition->type == DEFINITION_TYPE_DOES) {
            forth_printf("%s DOES> ", definition->name);
        } else {
            forth_printf(": %s\n  ", definition->name);
            code_p += 4;
        }
        for (int i = 0;; i += 2) {
            uint16_t p = code_p + i;
            uint16_t addr = *memory_at16(state->MEMORY, p);
            if (addr == state->BUILTINS[BUILTIN_WORD_EXIT]) {
                break;
            }
            if (addr == state->BUILTINS[BUILTIN_WORD_LIT]) {
                int16_t value = *memory_at16(state->MEMORY, p + 2);
                forth_printf("(LIT) %d ", value);
                i += 2;
            } else if (addr == state->BUILTINS[BUILTIN_WORD_BRANCH]) {
                int16_t value = *memory_at16(state->MEMORY, p + 2);
                forth_printf("BRANCH %d ", value);
                i += 2;
            } else if (addr == state->BUILTINS[BUILTIN_WORD_QUESTION_BRANCH]) {
                int16_t value = *memory_at16(state->MEMORY, p + 2);
                forth_printf("?BRANCH %d ", value);
                i += 2;
            } else {
                Definition *d = get_definition(state->MEMORY, TO_NAME(state->MEMORY, addr));
                if (d != 0 && *d->name != '\0') {
                    forth_printf("%s ", d->name);
                    free_definition(d);
                } else {
                    forth_printf("%d ", addr);
                }
            }
        }
        forth_printf(";");
        if (definition->is_immediate) {
            forth_printf(" IMMEDIATE");
        }
    } else if (definition->type == DEFINITION_TYPE_DEFERRED) {
        forth_printf("DEFERRED %s IS\n", definition->name);
        uint16_t next_definition_p = *memory_at16(state->MEMORY, definition->pfa);
        Definition *d = get_definition(state->MEMORY, next_definition_p);
        see(state, d);
        free_definition(d);
    } else {
        forth_printf("UNKNOWN %s", definition->name);
    }
}


int builtin_see(InterpreterState *state) {
    read_name(word, definition);
    forth_printf("\n");
    see(state, definition);
    free_definition(definition);
    free(word);
    return 0;
}

int builtin_breakpoint(InterpreterState *state) {
    read_name(word, definition);
    state->breakpoint = FROM_BODY(definition->pfa);
    free_definition(definition);
    free(word);
    return 0;
}

int builtin_noop(InterpreterState *state) {
    return 0;
}

int builtin_error_not_executable(InterpreterState *state) {
    return ERROR_WORD_NOT_EXECUTABLE;
}

BuiltinFunction BUILTINS[MAX_BUILTINS] = {
    [BUILTIN_WORD_LIT] = builtin_error_not_executable,
    [BUILTIN_WORD_STORE] = builtin_store,
    //[BUILTIN_WORD_SHARP] = builtin_sharp,
    //[BUILTIN_WORD_SHARP_GREATER] = builtin_sharp_greater,
    //[BUILTIN_WORD_SHARP_S] = builtin_sharp_s,
    //[BUILTIN_WORD_NUMBER_TIB] = builtin_number_tib,
    [BUILTIN_WORD_TICK] = builtin_tick,
    [BUILTIN_WORD_PAREN] = builtin_paren,
    [BUILTIN_WORD_TIMES] = builtin_times,
    [BUILTIN_WORD_TIMES_DIVIDE] = builtin_times_divide,
    [BUILTIN_WORD_TIMES_DIVIDE_MOD] = builtin_times_divide_mod,
    [BUILTIN_WORD_PLUS] = builtin_plus,
    [BUILTIN_WORD_PLUS_STORE] = builtin_plus_store,
    //[BUILTIN_WORD_PLUS_LOOP] = builtin_plus_loop,
    [BUILTIN_WORD_COMMA] = builtin_comma,
    [BUILTIN_WORD_MINUS] = builtin_minus,
    [BUILTIN_WORD_DASH_TRAILING] = builtin_dash_trailing,
    [BUILTIN_WORD_DOT] = builtin_dot,
    [BUILTIN_WORD_DOT_QUOTE] = builtin_dot_quote,
    [BUILTIN_WORD_DOT_PAREN] = builtin_dot_paren,
    [BUILTIN_WORD_DIVIDE] = builtin_divide,
    [BUILTIN_WORD_DIVIDE_MOD] = builtin_divide_mod,
    [BUILTIN_WORD_ZERO_LESS] = builtin_zero_less,
    [BUILTIN_WORD_ZERO_EQUALS] = builtin_zero_equals,
    [BUILTIN_WORD_ZERO_GREATER] = builtin_zero_greater,
    [BUILTIN_WORD_ONE_PLUS] = builtin_one_plus,
    [BUILTIN_WORD_ONE_MINUS] = builtin_one_minus,
    [BUILTIN_WORD_TWO_PLUS] = builtin_two_plus,
    [BUILTIN_WORD_TWO_MINUS] = builtin_two_minus,
    [BUILTIN_WORD_TWO_DIVIDE] = builtin_two_divide,
    [BUILTIN_WORD_COLON] = builtin_colon,
    [BUILTIN_WORD_SEMI_COLON] = builtin_semi_colon,
    [BUILTIN_WORD_LESS_THAN] = builtin_less_than,
    //[BUILTIN_WORD_LESS_SHARP] = builtin_less_sharp,
    [BUILTIN_WORD_EQUALS] = builtin_equals,
    [BUILTIN_WORD_GREATER_THAN] = builtin_greater_than,
    //[BUILTIN_WORD_TO_BODY] = builtin_to_body,
    //[BUILTIN_WORD_TO_IN] = builtin_to_in,
    [BUILTIN_WORD_TO_R] = builtin_to_r,
    [BUILTIN_WORD_QUESTION_DUP] = builtin_question_dup,
    [BUILTIN_WORD_FETCH] = builtin_fetch,
    [BUILTIN_WORD_ABORT] = builtin_abort,
    //[BUILTIN_WORD_ABORT_QUOTE] = builtin_abort_quote,
    [BUILTIN_WORD_ABS] = builtin_abs,
    [BUILTIN_WORD_ALLOT] = builtin_allot,
    [BUILTIN_WORD_AND] = builtin_and,
    //[BUILTIN_WORD_BASE] = builtin_base,
    //[BUILTIN_WORD_BEGIN] = builtin_begin,
    //[BUILTIN_WORD_BLK] = builtin_blk,
    //[BUILTIN_WORD_BLOCK] = builtin_block,
    //[BUILTIN_WORD_BUFFER] = builtin_buffer,
    [BUILTIN_WORD_C_STORE] = builtin_c_store,
    [BUILTIN_WORD_C_FETCH] = builtin_c_fetch,
    [BUILTIN_WORD_CMOVE] = builtin_cmove,
    [BUILTIN_WORD_CMOVE_UP] = builtin_cmove_up,
    [BUILTIN_WORD_COMPILE] = builtin_compile,
    [BUILTIN_WORD_CONSTANT] = builtin_constant,
    //[BUILTIN_WORD_CONVERT] = builtin_convert,
    [BUILTIN_WORD_COUNT] = builtin_count,
    [BUILTIN_WORD_CR] = builtin_cr,
    [BUILTIN_WORD_CREATE] = builtin_create,
    [BUILTIN_WORD_D_PLUS] = builtin_d_plus,
    [BUILTIN_WORD_D_LESS_THAN] = builtin_d_less_than,
    //[BUILTIN_WORD_DECIMAL] = builtin_decimal,
    //[BUILTIN_WORD_DEFINITIONS] = builtin_definitions,
    [BUILTIN_WORD_DEPTH] = builtin_depth,
    [BUILTIN_WORD_DNEGATE] = builtin_dnegate,
    //[BUILTIN_WORD_DO] = builtin_do,
    [BUILTIN_WORD_DOES] = builtin_does,
    [BUILTIN_WORD_DROP] = builtin_drop,
    [BUILTIN_WORD_DUP] = builtin_dup,
    //[BUILTIN_WORD_ELSE] = builtin_else,
    [BUILTIN_WORD_EMIT] = builtin_emit,
    [BUILTIN_WORD_EXECUTE] = builtin_execute,
    [BUILTIN_WORD_EXIT] = builtin_exit,
    [BUILTIN_WORD_EXPECT] = builtin_expect,
    [BUILTIN_WORD_FILL] = builtin_fill,
    [BUILTIN_WORD_FIND] = builtin_find,
    //[BUILTIN_WORD_FLUSH] = builtin_flush,
    [BUILTIN_WORD_FORGET] = builtin_forget,
    //[BUILTIN_WORD_FORTH] = builtin_forth,
    [BUILTIN_WORD_FORTH_83] = builtin_forth_83,
    [BUILTIN_WORD_HERE] = builtin_here,
    //[BUILTIN_WORD_HOLD] = builtin_hold,
    //[BUILTIN_WORD_I] = builtin_i,
    //[BUILTIN_WORD_IF] = builtin_if,
    [BUILTIN_WORD_IMMEDIATE] = builtin_immediate,
    //[BUILTIN_WORD_J] = builtin_j,
    [BUILTIN_WORD_KEY] = builtin_key,
    //[BUILTIN_WORD_LEAVE] = builtin_leave,
    [BUILTIN_WORD_LITERAL] = builtin_literal,
    //[BUILTIN_WORD_LOAD] = builtin_load,
    //[BUILTIN_WORD_LOOP] = builtin_loop,
    [BUILTIN_WORD_MAX] = builtin_max,
    [BUILTIN_WORD_MIN] = builtin_min,
    [BUILTIN_WORD_MOD] = builtin_mod,
    [BUILTIN_WORD_NEGATE] = builtin_negate,
    [BUILTIN_WORD_NOT] = builtin_not,
    [BUILTIN_WORD_OR] = builtin_or,
    [BUILTIN_WORD_OVER] = builtin_over,
    //[BUILTIN_WORD_PAD] = builtin_pad,
    [BUILTIN_WORD_PICK] = builtin_pick,
    [BUILTIN_WORD_QUIT] = builtin_quit,
    [BUILTIN_WORD_R_FROM] = builtin_r_from,
    [BUILTIN_WORD_R_FETCH] = builtin_r_fetch,
    //[BUILTIN_WORD_REPEAT] = builtin_repeat,
    [BUILTIN_WORD_ROLL] = builtin_roll,
    [BUILTIN_WORD_ROT] = builtin_rot,
    //[BUILTIN_WORD_SAVE_BUFFERS] = builtin_save_buffers,
    //[BUILTIN_WORD_SIGN] = builtin_sign,
    [BUILTIN_WORD_SPACE] = builtin_space,
    [BUILTIN_WORD_SPACES] = builtin_spaces,
    //[BUILTIN_WORD_SPAN] = builtin_span,
    //[BUILTIN_WORD_STATE] = builtin_state,
    [BUILTIN_WORD_SWAP] = builtin_swap,
    //[BUILTIN_WORD_THEN] = builtin_then,
    //[BUILTIN_WORD_TIB] = builtin_tib,
    [BUILTIN_WORD_TYPE] = builtin_type,
    [BUILTIN_WORD_U_DOT] = builtin_u_dot,
    [BUILTIN_WORD_U_LESS_THAN] = builtin_u_less_than,
    [BUILTIN_WORD_UM_TIMES] = builtin_um_times,
    [BUILTIN_WORD_UM_DIVIDE_MOD] = builtin_um_divide_mod,
    //[BUILTIN_WORD_UPDATE] = builtin_update,
    [BUILTIN_WORD_VARIABLE] = builtin_variable,
    //[BUILTIN_WORD_VOCABULARY] = builtin_vocabulary,
    //[BUILTIN_WORD_WHILE] = builtin_while,
    //[BUILTIN_WORD_WORD] = builtin_word,
    [BUILTIN_WORD_XOR] = builtin_xor,
    [BUILTIN_WORD_LEFT_BRACKET] = builtin_left_bracket,
    [BUILTIN_WORD_BRACKET_TICK] = builtin_bracket_tick,
    [BUILTIN_WORD_BRACKET_COMPILE] = builtin_bracket_compile,
    [BUILTIN_WORD_RIGHT_BRACKET] = builtin_right_bracket,

    [BUILTIN_WORD_TWO_STORE] = builtin_two_store,
    [BUILTIN_WORD_TWO_FETCH] = builtin_two_fetch,
    //[BUILTIN_WORD_TWO_CONSTANT] = builtin_two_constant,
    [BUILTIN_WORD_TWO_DROP] = builtin_two_drop,
    [BUILTIN_WORD_TWO_DUP] = builtin_two_dup,
    [BUILTIN_WORD_TWO_OVER] = builtin_two_over,
    [BUILTIN_WORD_TWO_ROT] = builtin_two_rot,
    [BUILTIN_WORD_TWO_SWAP] = builtin_two_swap,
    //[BUILTIN_WORD_TWO_VARIABLE] = builtin_two_variable,
    [BUILTIN_WORD_D_MINUS] = builtin_d_minus,
    [BUILTIN_WORD_D_DOT] = builtin_d_dot,
    [BUILTIN_WORD_D_DOT_R] = builtin_d_dot_r,
    [BUILTIN_WORD_D_ZERO_EQUALS] = builtin_d_zero_equals,
    [BUILTIN_WORD_D_TWO_DIVIDE] = builtin_d_two_divide,
    [BUILTIN_WORD_D_EQUAL] = builtin_d_equal,
    [BUILTIN_WORD_DABS] = builtin_dabs,
    [BUILTIN_WORD_DMAX] = builtin_dmax,
    [BUILTIN_WORD_DMIN] = builtin_dmin,
    [BUILTIN_WORD_DU_LESS_THAN] = builtin_du_less_than,

    [BUILTIN_WORD_BACKWARD_MARK] = builtin_backward_mark,
    [BUILTIN_WORD_BACKWARD_RESOLVE] = builtin_backward_resolve,
    [BUILTIN_WORD_FORWARD_MARK] = builtin_forward_mark,
    [BUILTIN_WORD_FORWARD_RESOLVE] = builtin_forward_resolve,

    [BUILTIN_WORD_QUESTION_BRANCH] = builtin_error_not_executable,
    [BUILTIN_WORD_BRANCH] = builtin_error_not_executable,

    //[BUILTIN_WORD_NEXT_BLOCK] = builtin_next_block,
    [BUILTIN_WORD_DOT_R] = builtin_dot_r,
    [BUILTIN_WORD_TWO_TIMES] = builtin_two_times,
    //[BUILTIN_WORD_BL] = builtin_bl,
    //[BUILTIN_WORD_BLANK] = builtin_blank,
    //[BUILTIN_WORD_C_COMMA] = builtin_c_comma,
    //[BUILTIN_WORD_DUMP] = builtin_dump,
    //[BUILTIN_WORD_EDITOR] = builtin_editor,
    //[BUILTIN_WORD_EMPTY_BUFFERS] = builtin_empty_buffers,
    //[BUILTIN_WORD_END] = builtin_end,
    //[BUILTIN_WORD_ERASE] = builtin_erase,
    //[BUILTIN_WORD_HEX] = builtin_hex,
    [BUILTIN_WORD_INTERPRET] = builtin_interpret,
    //[BUILTIN_WORD_K] = builtin_k,
    //[BUILTIN_WORD_LIST] = builtin_list,
    //[BUILTIN_WORD_OCTAL] = builtin_octal,
    //[BUILTIN_WORD_OFFSET] = builtin_offset,
    //[BUILTIN_WORD_QUERY] = builtin_query,
    //[BUILTIN_WORD_RECURSE] = builtin_recurse,
    //[BUILTIN_WORD_SCR] = builtin_scr,
    //[BUILTIN_WORD_SP_FETCH] = builtin_sp_fetch,
    //[BUILTIN_WORD_THRU] = builtin_thru,
    [BUILTIN_WORD_U_DOT_R] = builtin_u_dot_r,

    [BUILTIN_WORD_NOT_EQUAL] = builtin_not_equal,

    [BUILTIN_WORD_INCLUDE] = builtin_include,
    [BUILTIN_WORD_DASH_ROT] = builtin_dash_rot,
    [BUILTIN_WORD_TUCK] = builtin_tuck,
    [BUILTIN_WORD_NIP] = builtin_nip,
    [BUILTIN_WORD_SKIP] = builtin_skip,
    [BUILTIN_WORD_SCAN] = builtin_scan,
    [BUILTIN_WORD_ZERO_NOT_EQUAL] = builtin_zero_not_equal,
    [BUILTIN_WORD_BDOS] = builtin_bdos,
    [BUILTIN_WORD_INT10H] = builtin_int10h,
    [BUILTIN_WORD_LINEMOVE] = builtin_linemove,
    [BUILTIN_WORD_UPPER] = builtin_upper,
    [BUILTIN_WORD_DIGIT] = builtin_digit,
    [BUILTIN_WORD_RPICK] = builtin_rpick,
    [BUILTIN_WORD_SP_FETCH] = builtin_sp_fetch,
    [BUILTIN_WORD_RP_FETCH] = builtin_rp_fetch,
    [BUILTIN_WORD_SP_STORE] = builtin_sp_store,
    [BUILTIN_WORD_RP_STORE] = builtin_rp_store,
    [BUILTIN_WORD_U_TWO_DIVIDE] = builtin_u_two_divide,
    [BUILTIN_WORD_U_GREATER_THAN] = builtin_u_greater_than,
    [BUILTIN_WORD_S_TO_D] = builtin_s_to_d,
    [BUILTIN_WORD_COMP] = builtin_comp,
    [BUILTIN_WORD_CAPS_COMP] = builtin_caps_comp,
    [BUILTIN_WORD_LSHIFT] = builtin_lshift,
    [BUILTIN_WORD_RSHIFT] = builtin_rshift,
    [BUILTIN_WORD_M_TIMES] = builtin_m_times,
    [BUILTIN_WORD_M_TIMES_DIVIDE] = builtin_m_times_divide,
    [BUILTIN_WORD_FM_DIVIDE_MOD] = builtin_fm_divide_mod,
    [BUILTIN_WORD_SM_DIVIDE_REM] = builtin_sm_divide_rem,

    [BUILTIN_WORD_FINNISH] = builtin_finnish,

    [BUILTIN_WORD_SEE] = builtin_see,
    [BUILTIN_WORD_BREAKPOINT] = builtin_breakpoint,
};

int execute_builtin(InterpreterState *state, enum BuiltinWord word) {
    if (word >= MAX_BUILTINS || BUILTINS[word] == 0) return ERROR_WORD_NOT_FOUND;
    return BUILTINS[word](state);
}

void add_builtins(AddBuiltinFunction add_builtin) {
    add_builtin("(LIT)", BUILTIN_WORD_LIT, 0);
    add_builtin("!", BUILTIN_WORD_STORE, 0);
    add_builtin("#", BUILTIN_WORD_SHARP, 0);
    add_builtin("#>", BUILTIN_WORD_SHARP_GREATER, 0);
    add_builtin("#S", BUILTIN_WORD_SHARP_S, 0);
    //add_builtin("#TIB", BUILTIN_WORD_NUMBER_TIB, 0);
    add_builtin("'", BUILTIN_WORD_TICK, 0);
    add_builtin("(", BUILTIN_WORD_PAREN, 1);
    add_builtin("*", BUILTIN_WORD_TIMES, 0);
    add_builtin("*/", BUILTIN_WORD_TIMES_DIVIDE, 0);
    add_builtin("*/MOD", BUILTIN_WORD_TIMES_DIVIDE_MOD, 0);
    add_builtin("+", BUILTIN_WORD_PLUS, 0);
    add_builtin("+!", BUILTIN_WORD_PLUS_STORE, 0);
    add_builtin("+LOOP", BUILTIN_WORD_PLUS_LOOP, 1);
    add_builtin(",", BUILTIN_WORD_COMMA, 0);
    add_builtin("-", BUILTIN_WORD_MINUS, 0);
    add_builtin("-TRAILING", BUILTIN_WORD_DASH_TRAILING, 0);
    add_builtin(".", BUILTIN_WORD_DOT, 0);
    add_builtin(".\"", BUILTIN_WORD_DOT_QUOTE, 1);
    add_builtin(".(", BUILTIN_WORD_DOT_PAREN, 1);
    add_builtin("/", BUILTIN_WORD_DIVIDE, 0);
    add_builtin("/MOD", BUILTIN_WORD_DIVIDE_MOD, 0);
    add_builtin("0<", BUILTIN_WORD_ZERO_LESS, 0);
    add_builtin("0=", BUILTIN_WORD_ZERO_EQUALS, 0);
    add_builtin("0>", BUILTIN_WORD_ZERO_GREATER, 0);
    add_builtin("1+", BUILTIN_WORD_ONE_PLUS, 0);
    add_builtin("1-", BUILTIN_WORD_ONE_MINUS, 0);
    add_builtin("2+", BUILTIN_WORD_TWO_PLUS, 0);
    add_builtin("2-", BUILTIN_WORD_TWO_MINUS, 0);
    add_builtin("2/", BUILTIN_WORD_TWO_DIVIDE, 0);
    add_builtin(":", BUILTIN_WORD_COLON, 0);
    add_builtin(";", BUILTIN_WORD_SEMI_COLON, 1);
    add_builtin("<", BUILTIN_WORD_LESS_THAN, 0);
    add_builtin("<#", BUILTIN_WORD_LESS_SHARP, 0);
    add_builtin("=", BUILTIN_WORD_EQUALS, 0);
    add_builtin(">", BUILTIN_WORD_GREATER_THAN, 0);
    //add_builtin(">BODY", BUILTIN_WORD_TO_BODY, 0);
    //add_builtin(">IN", BUILTIN_WORD_TO_IN, 0);
    add_builtin(">R", BUILTIN_WORD_TO_R, 0);
    add_builtin("?DUP", BUILTIN_WORD_QUESTION_DUP, 0);
    add_builtin("@", BUILTIN_WORD_FETCH, 0);
    add_builtin("ABORT", BUILTIN_WORD_ABORT, 0);
    add_builtin("ABORT\"", BUILTIN_WORD_ABORT_QUOTE, 1);
    add_builtin("ABS", BUILTIN_WORD_ABS, 0);
    add_builtin("ALLOT", BUILTIN_WORD_ALLOT, 0);
    add_builtin("AND", BUILTIN_WORD_AND, 0);
    //add_builtin("BASE", BUILTIN_WORD_BASE, 0);
    add_builtin("BEGIN", BUILTIN_WORD_BEGIN, 1);
    //add_builtin("BLK", BUILTIN_WORD_BLK, 0);
    add_builtin("BLOCK", BUILTIN_WORD_BLOCK, 0);
    add_builtin("BUFFER", BUILTIN_WORD_BUFFER, 0);
    add_builtin("C!", BUILTIN_WORD_C_STORE, 0);
    add_builtin("C@", BUILTIN_WORD_C_FETCH, 0);
    add_builtin("CMOVE", BUILTIN_WORD_CMOVE, 0);
    add_builtin("CMOVE>", BUILTIN_WORD_CMOVE_UP, 0);
    //add_builtin("COMPILE", BUILTIN_WORD_COMPILE, 0);
    add_builtin("CONSTANT", BUILTIN_WORD_CONSTANT, 0);
    add_builtin("CONVERT", BUILTIN_WORD_CONVERT, 0);
    add_builtin("COUNT", BUILTIN_WORD_COUNT, 0);
    add_builtin("CR", BUILTIN_WORD_CR, 0);
    add_builtin("CREATE", BUILTIN_WORD_CREATE, 0);
    add_builtin("D+", BUILTIN_WORD_D_PLUS, 0);
    add_builtin("D<", BUILTIN_WORD_D_LESS_THAN, 0);
    add_builtin("DECIMAL", BUILTIN_WORD_DECIMAL, 0);
    add_builtin("DEFINITIONS", BUILTIN_WORD_DEFINITIONS, 0);
    add_builtin("DEPTH", BUILTIN_WORD_DEPTH, 0);
    add_builtin("DNEGATE", BUILTIN_WORD_DNEGATE, 0);
    add_builtin("DO", BUILTIN_WORD_DO, 1);
    add_builtin("DOES>", BUILTIN_WORD_DOES, 0); // Should this be immediate? Implemented in a different way in Forth-83 standard
    add_builtin("DROP", BUILTIN_WORD_DROP, 0);
    add_builtin("DUP", BUILTIN_WORD_DUP, 0);
    add_builtin("ELSE", BUILTIN_WORD_ELSE, 1);
    add_builtin("EMIT", BUILTIN_WORD_EMIT, 0);
    add_builtin("EXECUTE", BUILTIN_WORD_EXECUTE, 0);
    add_builtin("EXIT", BUILTIN_WORD_EXIT, 0);
    add_builtin("EXPECT", BUILTIN_WORD_EXPECT, 0);
    add_builtin("FILL", BUILTIN_WORD_FILL, 0);
    add_builtin("FIND", BUILTIN_WORD_FIND, 0);
    add_builtin("FLUSH", BUILTIN_WORD_FLUSH, 0);
    add_builtin("FORGET", BUILTIN_WORD_FORGET, 0);
    add_builtin("FORTH", BUILTIN_WORD_FORTH, 0);
    add_builtin("FORTH-83", BUILTIN_WORD_FORTH_83, 0);
    add_builtin("HERE", BUILTIN_WORD_HERE, 0);
    add_builtin("HOLD", BUILTIN_WORD_HOLD, 0);
    add_builtin("I", BUILTIN_WORD_I, 0);
    add_builtin("IF", BUILTIN_WORD_IF, 1);
    add_builtin("IMMEDIATE", BUILTIN_WORD_IMMEDIATE, 0);
    add_builtin("J", BUILTIN_WORD_J, 0);
    add_builtin("KEY", BUILTIN_WORD_KEY, 0);
    add_builtin("LEAVE", BUILTIN_WORD_LEAVE, 1);
    add_builtin("LITERAL", BUILTIN_WORD_LITERAL, 1);
    add_builtin("LOAD", BUILTIN_WORD_LOAD, 0);
    add_builtin("LOOP", BUILTIN_WORD_LOOP, 1);
    add_builtin("MAX", BUILTIN_WORD_MAX, 0);
    add_builtin("MIN", BUILTIN_WORD_MIN, 0);
    add_builtin("MOD", BUILTIN_WORD_MOD, 0);
    add_builtin("NEGATE", BUILTIN_WORD_NEGATE, 0);
    add_builtin("NOT", BUILTIN_WORD_NOT, 0);
    add_builtin("OR", BUILTIN_WORD_OR, 0);
    add_builtin("OVER", BUILTIN_WORD_OVER, 0);
    add_builtin("PAD", BUILTIN_WORD_PAD, 0);
    add_builtin("PICK", BUILTIN_WORD_PICK, 0);
    add_builtin("QUIT", BUILTIN_WORD_QUIT, 0);
    add_builtin("R>", BUILTIN_WORD_R_FROM, 0);
    add_builtin("R@", BUILTIN_WORD_R_FETCH, 0);
    add_builtin("REPEAT", BUILTIN_WORD_REPEAT, 1);
    add_builtin("ROLL", BUILTIN_WORD_ROLL, 0);
    add_builtin("ROT", BUILTIN_WORD_ROT, 0);
    add_builtin("SAVE-BUFFERS", BUILTIN_WORD_SAVE_BUFFERS, 0);
    add_builtin("SIGN", BUILTIN_WORD_SIGN, 0);
    add_builtin("SPACE", BUILTIN_WORD_SPACE, 0);
    add_builtin("SPACES", BUILTIN_WORD_SPACES, 0);
    //add_builtin("SPAN", BUILTIN_WORD_SPAN, 0);
    //add_builtin("STATE", BUILTIN_WORD_STATE, 0);
    add_builtin("SWAP", BUILTIN_WORD_SWAP, 0);
    add_builtin("THEN", BUILTIN_WORD_THEN, 1);
    //add_builtin("TIB", BUILTIN_WORD_TIB, 0);
    add_builtin("TYPE", BUILTIN_WORD_TYPE, 0);
    add_builtin("U.", BUILTIN_WORD_U_DOT, 0);
    add_builtin("U<", BUILTIN_WORD_U_LESS_THAN, 0);
    add_builtin("UM*", BUILTIN_WORD_UM_TIMES, 0);
    add_builtin("UM/MOD", BUILTIN_WORD_UM_DIVIDE_MOD, 0);
    add_builtin("UPDATE", BUILTIN_WORD_UPDATE, 0);
    add_builtin("VARIABLE", BUILTIN_WORD_VARIABLE, 0);
    add_builtin("VOCABULARY", BUILTIN_WORD_VOCABULARY, 0);
    add_builtin("WHILE", BUILTIN_WORD_WHILE, 1);
    add_builtin("WORD", BUILTIN_WORD_WORD, 0);
    add_builtin("XOR", BUILTIN_WORD_XOR, 0);
    add_builtin("[", BUILTIN_WORD_LEFT_BRACKET, 1);
    add_builtin("[']", BUILTIN_WORD_BRACKET_TICK, 1);
    add_builtin("[COMPILE]", BUILTIN_WORD_BRACKET_COMPILE, 1);
    add_builtin("]", BUILTIN_WORD_RIGHT_BRACKET, 0);

    // Double words
    add_builtin("2!", BUILTIN_WORD_TWO_STORE, 0);
    add_builtin("2@", BUILTIN_WORD_TWO_FETCH, 0);
    //add_builtin("2CONSTANT", BUILTIN_WORD_TWO_CONSTANT, 0);
    add_builtin("2DROP", BUILTIN_WORD_TWO_DROP, 0);
    add_builtin("2DUP", BUILTIN_WORD_TWO_DUP, 0);
    add_builtin("2OVER", BUILTIN_WORD_TWO_OVER, 0);
    add_builtin("2ROT", BUILTIN_WORD_TWO_ROT, 0);
    add_builtin("2SWAP", BUILTIN_WORD_TWO_SWAP, 0);
    //add_builtin("2VARIABLE", BUILTIN_WORD_TWO_VARIABLE, 0);
    add_builtin("D-", BUILTIN_WORD_D_MINUS, 0);
    add_builtin("D.", BUILTIN_WORD_D_DOT, 0);
    add_builtin("D.R", BUILTIN_WORD_D_DOT_R, 0);
    add_builtin("D0=", BUILTIN_WORD_D_ZERO_EQUALS, 0);
    add_builtin("D2/", BUILTIN_WORD_D_TWO_DIVIDE, 0);
    add_builtin("D=", BUILTIN_WORD_D_EQUAL, 0);
    add_builtin("DABS", BUILTIN_WORD_DABS, 0);
    add_builtin("DMAX", BUILTIN_WORD_DMAX, 0);
    add_builtin("DMIN", BUILTIN_WORD_DMIN, 0);
    add_builtin("DU<", BUILTIN_WORD_DU_LESS_THAN, 0);

    // Marks and resolves
    add_builtin("<MARK", BUILTIN_WORD_BACKWARD_MARK, 0);
    add_builtin("<RESOLVE", BUILTIN_WORD_BACKWARD_RESOLVE, 0);
    add_builtin(">MARK", BUILTIN_WORD_FORWARD_MARK, 0);
    add_builtin(">RESOLVE", BUILTIN_WORD_FORWARD_RESOLVE, 0);
    //add_builtin("CONTEXT", BUILTIN_WORD_CONTEXT, 0);
    //add_builtin("CURRENT", BUILTIN_WORD_CURRENT, 0);

    // Branches
    add_builtin("BRANCH", BUILTIN_WORD_BRANCH, 0);
    add_builtin("?BRANCH", BUILTIN_WORD_QUESTION_BRANCH, 0);

    // Controlled reference words
    add_builtin("-->", BUILTIN_WORD_NEXT_BLOCK, 1);
    add_builtin(".R", BUILTIN_WORD_DOT_R, 0);
    add_builtin("2*", BUILTIN_WORD_TWO_TIMES, 0);
    //add_builtin("BL", BUILTIN_WORD_BL, 0);
    add_builtin("BLANK", BUILTIN_WORD_BLANK, 0);
    add_builtin("C,", BUILTIN_WORD_C_COMMA, 0);
    add_builtin("DUMP", BUILTIN_WORD_DUMP, 0);
    add_builtin("EDITOR", BUILTIN_WORD_EDITOR, 0);
    add_builtin("EMPTY-BUFFERS", BUILTIN_WORD_EMPTY_BUFFERS, 0);
    add_builtin("END", BUILTIN_WORD_END, 1);
    add_builtin("ERASE", BUILTIN_WORD_ERASE, 0);
    add_builtin("HEX", BUILTIN_WORD_HEX, 0);
    add_builtin("INTERPRET", BUILTIN_WORD_INTERPRET, 0);
    add_builtin("K", BUILTIN_WORD_K, 1);
    add_builtin("LIST", BUILTIN_WORD_LIST, 0);
    add_builtin("OCTAL", BUILTIN_WORD_OCTAL, 0);
    add_builtin("OFFSET", BUILTIN_WORD_OFFSET, 0);
    add_builtin("QUERY", BUILTIN_WORD_QUERY, 0);
    add_builtin("RECURSE", BUILTIN_WORD_RECURSE, 1);
    add_builtin("SCR", BUILTIN_WORD_SCR, 0);
    add_builtin("SP@", BUILTIN_WORD_SP_FETCH, 0);
    add_builtin("THRU", BUILTIN_WORD_THRU, 1);
    add_builtin("U.R", BUILTIN_WORD_U_DOT_R, 0);

    // Uncrontrolled reference words
    add_builtin("<>", BUILTIN_WORD_NOT_EQUAL, 0);

    // Additional words
    add_builtin("INCLUDE", BUILTIN_WORD_INCLUDE, 0);
    add_builtin("-ROT", BUILTIN_WORD_DASH_ROT, 0);
    add_builtin("TUCK", BUILTIN_WORD_TUCK, 0);
    add_builtin("NIP", BUILTIN_WORD_NIP, 0);
    add_builtin("SKIP", BUILTIN_WORD_SKIP, 0);
    add_builtin("SCAN", BUILTIN_WORD_SCAN, 0);
    add_builtin("0<>", BUILTIN_WORD_ZERO_NOT_EQUAL, 0);
    add_builtin("BDOS", BUILTIN_WORD_BDOS, 0);
    add_builtin("INT10H", BUILTIN_WORD_INT10H, 0);
    add_builtin("LINEMOVE", BUILTIN_WORD_LINEMOVE, 0);
    add_builtin("UPPER", BUILTIN_WORD_UPPER, 0);
    add_builtin("DIGIT", BUILTIN_WORD_DIGIT, 0);
    add_builtin("RPICK", BUILTIN_WORD_RPICK, 0);
    add_builtin("RP@", BUILTIN_WORD_RP_FETCH, 0);
    add_builtin("SP!", BUILTIN_WORD_SP_STORE, 0);
    add_builtin("RP!", BUILTIN_WORD_RP_STORE, 0);
    add_builtin("U2/", BUILTIN_WORD_U_TWO_DIVIDE, 0);
    add_builtin("U>", BUILTIN_WORD_U_GREATER_THAN, 0);
    add_builtin("S>D", BUILTIN_WORD_S_TO_D, 0);
    add_builtin("COMP", BUILTIN_WORD_COMP, 0);
    add_builtin("CAPS-COMP", BUILTIN_WORD_CAPS_COMP, 0);
    add_builtin("LSHIFT", BUILTIN_WORD_LSHIFT, 0);
    add_builtin("RSHIFT", BUILTIN_WORD_RSHIFT, 0);
    add_builtin("M*", BUILTIN_WORD_M_TIMES, 0);
    add_builtin("M*/", BUILTIN_WORD_M_TIMES_DIVIDE, 0);
    add_builtin("FM/MOD", BUILTIN_WORD_FM_DIVIDE_MOD, 0);
    add_builtin("SM/REM", BUILTIN_WORD_SM_DIVIDE_REM, 0);

    add_builtin("FINNISH!", BUILTIN_WORD_FINNISH, 0);

    // Debug words
    add_builtin("SEE", BUILTIN_WORD_SEE, 0);
    add_builtin("BREAKPOINT", BUILTIN_WORD_BREAKPOINT, 0);
}