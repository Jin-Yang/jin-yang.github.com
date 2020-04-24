#include <errno.h>
#include <assert.h>
#include <stdlib.h>

#include "heap.h"

static inline unsigned int _child_left(const unsigned int idx)
{
	return idx * 2 + 1;
}

static inline unsigned int _child_right(const unsigned int idx)
{
	return idx * 2 + 2;
}

static inline unsigned int _parent(const unsigned int idx)
{
	return (idx - 1) / 2;
}

static inline unsigned int _node_max(struct heap *h)
{
	if (h->count <= 1)
		return 0;
	return (h->count - 2) / 2;
}

static inline void _swap(struct heap *h, const unsigned int i1, const unsigned int i2)
{
	uintptr_t tmp;
	assert(i1 < h->count && i1 < h->count);

	tmp = h->data[i1];
	h->data[i1] = h->data[i2];
	h->data[i2] = tmp;
}

struct heap *heap_create(int (*cmp)(const uintptr_t, const uintptr_t))
{
	struct heap *h;

	if (cmp == NULL)
		return NULL;

	h = (struct heap *)malloc(sizeof(*h));
	if (h == NULL)
		return NULL;
	h->compare = cmp;
	h->count = 0;
	h->size = HEAP_DEFAULT_SIZE;
	h->data = malloc(sizeof(uintptr_t) * h->size);
	if (h->data == NULL) {
		heap_destroy(h);
		return NULL;
	}

        return h;
}

void heap_destroy(struct heap *h)
{
	if (h == NULL)
		return;
	if (h->data != NULL)
		free(h->data);
	free(h);
}

static int _pushup(struct heap * h, unsigned int idx)
{
	unsigned int parent;

	/* 0 is the root node */
	while (0 != idx) {
		parent = _parent(idx);

		log_info("------- %d(%ld)  %d(%ld)", idx, h->data[idx], parent, h->data[parent]);
		/* we are smaller than the parent */
		if (h->compare(h->data[idx], h->data[parent]) < 0) {
			log_info("------- swap");
			_swap(h, idx, parent);
		} else {
			log_info("------- return");
			return 0;
		}
		idx = parent;
	}

	return 0;
}

static int _pushdown(struct heap * h, unsigned int idx)
{
	unsigned int left, right, child;

	while (1) {
		left = _child_left(idx);
		right = _child_right(idx);

		if (right >= h->count) {
			/* can't pushdown any further */
			if (left >= h->count)
				return 0;
			child = left;
		} else if (h->compare(h->data[left], h->data[right]) < 0) {
			/* find smallest child */
			child = left;
		} else {
			child = right;
		}

		/* idx is smaller than child */
		if (h->compare(h->data[idx], h->data[child]) < 0)
			return 0;
		_swap(h, idx, child);
		idx = child;
	}

	return 0;
}

static int heap_verify_node(struct heap *h, unsigned int idx)
{
	unsigned int left, right;

	log_debug("verify node %u.", idx);
	left = _child_left(idx);
	if (left >= h->count)
		return HEAP_OK;
	if (h->compare(h->data[idx], h->data[left]) > 0)
		return HEAP_NOT_OK;

	right = _child_right(idx);
	if (right >= h->count)
		return HEAP_OK;
	if (h->compare(h->data[idx], h->data[right]) > 0)
		return HEAP_NOT_OK;

	return HEAP_OK;
}

int heap_verify(struct heap *h)
{
	unsigned int idx;

	if (h == NULL)
		return -1;

	log_info("node max %d/%d", _node_max(h), h->count);
	for (idx = _node_max(h); idx > 0; idx--)
		if (heap_verify_node(h, idx) == HEAP_NOT_OK)
			return HEAP_NOT_OK;
	return heap_verify_node(h, 0);
}

int heap_insert(struct heap *h, uintptr_t v)
{
	uintptr_t *tmp;
	unsigned int size;

	if (h == NULL)
		return -EINVAL;

	if (h->count >= h->size) {
		size = h->size * 2;
		tmp = realloc(h->data, size);
		if (tmp == NULL)
			return -ENOMEM;
		h->data = tmp;
		h->size = size;
	}
	h->data[h->count] = v;

	return _pushup(h, h->count++);
}

/* NOTE: valid only when value is a ptr */
int heap_remove(struct heap *h, uintptr_t v)
{
	unsigned int idx;

	if (h == NULL)
		return -EINVAL;

	for (idx = 0; idx < h->count; idx++)
		if (h->data[idx] == v)
			break;
	if (idx == h->count)
		return -ENOENT;

	/* swap the item we found with the last item on the heap */
	h->data[idx] = h->data[h->count - 1];
	h->count--;
	_pushup(h, idx); /* ensure heap property */

	return 0;
}

int heap_get_root(struct heap *h, uintptr_t *ret)
{
	uintptr_t tmp;

	if (h == NULL || ret == NULL)
		return -EINVAL;
	if (heap_count(h) == 0)
		return -1;

	tmp = h->data[0];
	h->data[0] = h->data[h->count - 1];
	h->count--;
	if (h->count > 1)
		_pushdown(h, 0);
	*ret = tmp;
	return 0;
}
