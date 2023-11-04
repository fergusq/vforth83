#ifndef STACK_H
#define STACK_H

typedef struct _Stack {
    uint16_t *bottom;
    uint16_t *top;
    int size;
    int allocated_size;
} Stack;

Stack *create_stack();

int pop(Stack *stack, uint16_t *ret);

int pick(Stack *stack, uint16_t pos, uint16_t *ret);

void push(Stack *stack, uint16_t value);

void free_stack(Stack *stack);

#endif