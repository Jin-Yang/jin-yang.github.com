#ifndef LIBUTILS_STRUCT_HEAP_H_
#define LIBUTILS_STRUCT_HEAP_H_ 1

#include <stdint.h>

#define HEAP_DEFAULT_SIZE   16
#define HEAP_OK              0
#define HEAP_NOT_OK         -1


#include <stdio.h>

#define log_debug(...)  do { printf("debug: " __VA_ARGS__); putchar('\n'); } while(0);
#define log_info(...)   do { printf("info : " __VA_ARGS__); putchar('\n'); } while(0);
#define log_error(...)  do { printf("error: " __VA_ARGS__); putchar('\n'); } while(0);

/*
 * index #0 is the root node.
 */
struct heap {
	int (*compare)(const uintptr_t, const uintptr_t);

	uintptr_t *data;
	unsigned int count, size;
};

static inline int heap_count(struct heap *h)
{
	return h->count;
}

static inline int heap_size(struct heap *h)
{
	return h->size;
}

void heap_destroy(struct heap *h);
struct heap *heap_create(int (*cmp)(const uintptr_t, const uintptr_t));

int heap_verify(struct heap *h);
int heap_insert(struct heap *h, uintptr_t v);
int heap_remove(struct heap *h, uintptr_t v);
int heap_get_root(struct heap *h, uintptr_t *ret);

#endif
