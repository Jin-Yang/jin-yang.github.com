#ifndef LIBUTILS_STRUCT_STACK_H_
#define LIBUTILS_STRUCT_STACK_H_ 1

#include <stdint.h>

struct stack {
        int top, capacity;
	uintptr_t *data;
};

static inline int stack_is_full(struct stack *s)
{
        return (s->top >= (s->capacity - 1));
}

static inline int stack_is_empty(struct stack *s)
{
        return (s->top <= -1);
}

static inline int stack_get_size(struct stack *s)
{
        return s->top + 1;
}

static inline int stack_get_capacity(struct stack *s)
{
        return s->capacity;
}

void stack_destroy(struct stack *s);
struct stack *stack_create(int capacity);
int stack_push(struct stack *s, uintptr_t val);
int stack_pop(struct stack *s, uintptr_t *ret);
int stack_top(struct stack *s, uintptr_t *ret);

#endif
