#include <assert.h>
#include <stdlib.h>

#include "double.h"

struct dlist_header *dlist_header_create(void)
{
	return calloc(1, sizeof(struct dlist_header));
}

void dlist_header_destroy(struct dlist_header *header)
{
	if (header == NULL)
		return;
	free(header);
}

void dlist_destroy(struct dlist_header *list, void (*func)(void *))
{
	struct dlist_node *curr, *next;

	assert(list != NULL);
	for (curr = list->head; curr; curr = next) {
		next = curr->next;
		if (func)
			(*func)(curr->data);
		free(curr);
	}
	free(list);
}

void dlist_del_node(struct dlist_header *list, struct dlist_node *node, void (*func)(void *))
{
	assert(list != NULL && node != NULL);
	if (list->head == node) {
		list->head = node->next;
		if (list->head == NULL)
			list->tail = NULL;
		else
			node->next->prev = NULL;
	} else if (list->tail == node) {
		list->tail = node->prev;
		if (list->tail == NULL)
			list->head = NULL;
		else
			node->prev->next = NULL;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	if (func)
		(*func)(node->data);
	free(node);
	list->size--;
}

struct dlist_node *dlist_add_tail(struct dlist_header *list, void *data)
{
	struct dlist_node *node;

	assert(list != NULL && data != NULL);
	node = (struct dlist_node *)malloc(sizeof(*node));
	if (node == NULL)
		return NULL;

	node->next = NULL;
	node->data = data;
	if (list->tail == NULL) {
		list->head = node;
		node->prev = NULL;
	} else {
		list->tail->next = node;
		node->prev = list->tail;
	}
	list->tail = node;
	list->size++;

	return node;
}

struct dlist_node *dlist_add_head(struct dlist_header *list, void *data)
{
	struct dlist_node *node;

	assert(list != NULL && data != NULL);
	node = (struct dlist_node *)malloc(sizeof(*node));
	if (node == NULL)
		return NULL;
	node->data = data;

	node->prev = NULL;
	if (list->head == NULL) {
		list->tail = node;
		node->next = NULL;
	} else {
		list->head->prev = node;
		node->next = list->head;
	}
	list->head = node;
	list->size++;

	return node;
}
