#ifndef LIBUTILS_STRUCT_SKIPLIST_H_
#define LIBUTILS_STRUCT_SKIPLIST_H_ 1

#include <stdint.h>

/* should be enough for 2^64 elements */
#define SKIPLIST_MAX_LEVEL 64
#define list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

struct skiplist_list {
        struct skiplist_list *prev, *next;
};

struct skiplist {
	int (*compare)(const uintptr_t, const uintptr_t);

	int level, count;
        struct skiplist_list head[SKIPLIST_MAX_LEVEL];
};

struct skipnode {
	uintptr_t key, value;
        struct skiplist_list list[0];
};

static inline int skiplist_get_count(const struct skiplist *s)
{
	return s->count;
}

struct skiplist *skiplist_create(int (*cmp)(const uintptr_t, const uintptr_t));
void skiplist_destroy(struct skiplist *s);

void skiplist_remove(struct skiplist *s, uintptr_t key);
struct skipnode *skiplist_search(struct skiplist *s, uintptr_t key);
struct skipnode *skiplist_insert(struct skiplist *s, uintptr_t key, uintptr_t value);

#endif
