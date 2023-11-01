#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

#include "stack.h"

void print_stack(Stack *stack) {
    printf("--- BOTTOM [\n");
    for (int i = 0; i < stack->size; i++) {
        printf("%d\n", stack->bottom[i]);
    }
    printf("--- ] size = %d\n", stack->size);
}

Stack *create_stack() {
    Stack *stack = malloc(sizeof(*stack));
    stack->bottom = malloc(sizeof(uint16_t));
    stack->top = stack->bottom - 1;
    stack->size = 0;
    stack->allocated_size = 1;
    return stack;
}

int pop(Stack *stack, uint16_t *ret) {
    if (stack->size == 0) {
        return -1;
    } else {
        *ret = *stack->top;
        stack->top -= 1;
        stack->size -= 1;
        return stack->size;
    }
}

int pick(Stack *stack, uint16_t pos, uint16_t *ret) {
    if (pos >= stack->size) {
        return -1;
    } else {
        *ret = stack->bottom[stack->size - pos - 1];
        return 0;
    }
}

void push(Stack *stack, uint16_t value) {
    if (stack->size == stack->allocated_size) {
        stack->allocated_size *= 2;
        stack->bottom = realloc(stack->bottom, stack->allocated_size*sizeof(uint16_t));
    }
    stack->size += 1;
    stack->top = &stack->bottom[stack->size - 1];
    *stack->top = value;
    //print_stack(stack);
}

void free_stack(Stack *stack) {
    free(stack->bottom);
    free(stack);
}