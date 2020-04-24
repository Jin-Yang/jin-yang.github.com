#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "stack.h"

void stack_destroy(struct stack *s)
{
	if (s == NULL)
		return;
	if (s->data != NULL)
		free(s->data);
	free(s);
}

struct stack *stack_create(int capacity)
{
	struct stack *s;

	if (capacity <= 0)
		capacity = 64;

	s = (struct stack *)malloc(sizeof(*s));
	if (s == NULL)
		return NULL;

	s->data = malloc(sizeof(uintptr_t) * capacity);
	if (s->data == NULL) {
		free(s);
		return NULL;
	}
	s->top = -1;
	s->capacity = capacity;

	return s;
}

static int stack_double_capacity(struct stack *s)
{
	int cap;
	uintptr_t *tmp;

	cap = s->capacity * 2;
	tmp = realloc(s->data, sizeof(uintptr_t) * cap);
	if (tmp == NULL)
		return -ENOMEM;
	s->data = tmp;
	s->capacity = cap;

	return 0;
}

int stack_push(struct stack *s, uintptr_t val)
{
	int rc;

	if (stack_is_full(s)) {
		rc = stack_double_capacity(s);
		if (rc < 0)
			return rc;
	}
	s->data[++(s->top)] = val;
	return 0;
}

int stack_pop(struct stack *s, uintptr_t *ret)
{
	uintptr_t val;

	if (stack_is_empty(s))
		return -1;
	val = s->data[(s->top)--];
	if (ret != NULL)
		*ret = val;
	return 0;
}

int stack_top(struct stack *s, uintptr_t *ret)
{
	if (ret == NULL)
		return -EINVAL;
	if (stack_is_empty(s))
		return -1;
	*ret = s->data[s->top];
	return 0;
}
