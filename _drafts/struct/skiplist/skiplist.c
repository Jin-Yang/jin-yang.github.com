#include "skiplist.h"

#include <stdlib.h>

#define SASIZE(arr)  (int)(sizeof(arr)/sizeof((arr)[0]))

static inline void list_init(struct skiplist_list *list)
{
	list->prev = list;
	list->next = list;
}

static inline void list_do_add(struct skiplist_list *list, struct skiplist_list *prev, struct skiplist_list *next)
{
	list->next = next;
	list->prev = prev;
	next->prev = list;
	prev->next = list;
}

static inline void list_do_del(struct skiplist_list *prev, struct skiplist_list *next)
{
	prev->next = next;
	next->prev = prev;
}

static inline void list_add(struct skiplist_list *list, struct skiplist_list *prev)
{
	list_do_add(list, prev, prev->next);
}

static inline void list_del(struct skiplist_list *link)
{
	list_do_del(link->prev, link->next);
	list_init(link);
}

static inline int list_empty(struct skiplist_list *link)
{
	return link->next == link;
}

struct skiplist *skiplist_create(int (*cmp)(const uintptr_t, const uintptr_t))
{
	int i;
	struct skiplist *s;

	if (cmp == NULL)
		return NULL;

	s = (struct skiplist *)malloc(sizeof(*s));
	if (s == NULL)
		return NULL;
	for (i = 0; i < SASIZE(s->head); i++)
		list_init(&s->head[i]);
	s->level = 1;
	s->count = 0;
	s->compare = cmp;
	return s;
}

void skiplist_destroy(struct skiplist *s)
{
	struct skipnode *node;
	struct skiplist_list *pos, *end, *ptr;

	end = &s->head[0];
	pos = s->head[0].next;
	for (ptr = pos->next; pos != end; pos = ptr, ptr = pos->next) {
		node = list_entry(pos, struct skipnode, list[0]);
		free(node);
	}
	free(s);
}

static int random_level(void)
{
	int level = 1;

	while ((random() & 0xffff) < 0xffff * 0.25)
		level++;
	return level > SKIPLIST_MAX_LEVEL ? SKIPLIST_MAX_LEVEL : level;
}

static struct skipnode *skipnode_create(int level, int key, int value)
{
	struct skipnode *node;

	node = malloc(sizeof(*node) + level * sizeof(struct skiplist_list));
	if (node == NULL)
		return NULL;
	node->key = key;
	node->value = value;
	return node;
}

struct skipnode *skiplist_insert(struct skiplist *s, uintptr_t key, uintptr_t value)
{
	int level, i;
	struct skipnode *node, *tmp;
	struct skiplist_list *pos, *end;

	level = random_level();
	if (level > s->level)
		s->level = level;
	node = skipnode_create(level, key, value);
	if (node == NULL)
		return NULL;
	for (i = s->level - 1, pos = &s->head[i], end = pos; i >= 0; i--) {
		for (pos = pos->next; pos != end; pos = pos->next) {
			tmp = list_entry(pos, struct skipnode, list[i]);
			if (s->compare(tmp->key, key) >= 0) {
				end = &tmp->list[i];
				break;
			}
		}
		pos = end->prev;
		if (i < level)
			list_do_add(&node->list[i], pos, end);
		pos--;
		end--;
	}
	s->count++;

	return node;
}

struct skipnode *skiplist_search(struct skiplist *s, uintptr_t key)
{
	int i;
	struct skipnode *node = NULL;
	struct skiplist_list *pos, *end;

	for (i = s->level - 1, pos = &s->head[i], end = pos; i >= 0; i--) {
		for (pos = pos->next; pos != end; pos = pos->next) {
			node = list_entry(pos, struct skipnode, list[i]);
			if (s->compare(node->key, key) >= 0) {
				end = &node->list[i];
				break;
			}
		}

		if (node != NULL && s->compare(node->key, key) == 0)
			return node;
		pos = end->prev;
		pos--;
		end--;
	}

	return NULL;
}

static void _remove(struct skiplist *s, struct skipnode *node, int level)
{
	int i;

	for (i = 0; i < level; i++) {
		list_del(&node->list[i]);
		if (list_empty(&s->head[i]))
			s->level--;
	}
	free(node);
	s->count--;
}

void skiplist_remove(struct skiplist *s, uintptr_t key)
{
	int i;
	struct skipnode *node;
	struct skiplist_list *pos, *end, *ptr;

	for (i = s->level - 1, pos = &s->head[i], end = pos; i >= 0; i--) {
		pos = pos->next;
		for (ptr = pos->next; pos != end; pos = ptr, ptr = pos->next) {
			node = list_entry(pos, struct skipnode, list[i]);
			if (s->compare(node->key, key) > 0) {
				end = &node->list[i];
				break;
			} else if (s->compare(node->key, key) == 0) {
				/* we allow nodes with same key. */
				_remove(s, node, i + 1);
			}
		}
		pos = end->prev;
		pos--;
		end--;
	}
}

