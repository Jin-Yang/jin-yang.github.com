
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

llentry_t *llentry_create(char *key, void *value)
{
        llentry_t *e;

        e = (llentry_t *)malloc(sizeof(*e));
        if (e) {
                e->key = key;
                e->value = value;
                e->next = NULL;
        }

        return e;
}

void llentry_destroy(llentry_t *e)
{
        free(e);
}

llist_t *llist_create(void)
{
        llist_t *ret;

        ret = (llist_t *)calloc(1, sizeof(*ret));
        if (ret == NULL)
                return NULL;

        return ret;
}

void llist_destroy(llist_t *l)
{
        llentry_t *e_this;
        llentry_t *e_next;

        if (l == NULL)
                return;

        for (e_this = l->head; e_this != NULL; e_this = e_next) {
                e_next = e_this->next;
                llentry_destroy(e_this);
        }

        free(l);
}

void llist_append(llist_t *l, llentry_t *e)
{
        e->next = NULL;

        if (l->tail == NULL)
                l->head = e;
        else
                l->tail->next = e;

        l->tail = e;

        ++(l->size);
}

void llist_prepend(llist_t *l, llentry_t *e)
{
        e->next = l->head;
        l->head = e;

        if (l->tail == NULL)
                l->tail = e;

        ++(l->size);
}

void llist_remove(llist_t *l, llentry_t *e)
{
        llentry_t *prev;

        if ((l == NULL) || (e == NULL))
                return;

        prev = l->head;
        while ((prev != NULL) && (prev->next != e))
                prev = prev->next;

        if (prev != NULL)
                prev->next = e->next;
        if (l->head == e)
                l->head = e->next;
        if (l->tail == e)
                l->tail = prev;

        --(l->size);
}

int llist_size(llist_t *l)
{
        return (l ? l->size : -1);
}

static int llist_strcmp(llentry_t *e, void *ud)
{
        if ((e == NULL) || (ud == NULL))
                return -1;
        return (strcmp(e->key, (const char *)ud));
}

llentry_t *llist_search_custom(llist_t *l, int (*compare)(llentry_t *, void *), void *user_data)
{
        llentry_t *e;

        if (l == NULL)
                return NULL;

        e = l->head;
        while (e != NULL) {
                llentry_t *next = e->next;

                if (compare(e, user_data) == 0)
                        break;

                e = next;
        }

        return e;
}

llentry_t *llist_search(llist_t *l, const char *key)
{
        return (llist_search_custom(l, llist_strcmp, (void *)key));
}

llentry_t *llist_head(llist_t *l)
{
        if (l == NULL)
                return NULL;
        return (l->head);
}

llentry_t *llist_tail(llist_t *l)
{
        if (l == NULL)
                return NULL;
        return (l->tail);
}





















struct slnode *slnode_create(char *key, void *value)
{
        struct slnode *node;

        node = (struct slnode *)malloc(sizeof(*node));
        if (node) {
                node->key = key;
                node->value = value;
                node->next = NULL;
        }

        return node;
}

void slnode_destory(struct slnode *node)
{
        free(node);
}

struct slist *slist_create(void)
{
        return (struct slist *)calloc(1, sizeof(struct slist));
}

void slist_destroy(struct slist *list)
{
	struct slnode *this, *next;

        if (list == NULL)
                return;

        for (this = list->head; this != NULL; this = next) {
                next = this->next;
                slnode_destory(this);
        }

        free(list);
}

struct slnode *slist_head(struct slist *list)
{
        if (list == NULL)
                return NULL;
        return (list->head);
}

struct slnode *slist_tail(struct slist *list)
{
        if (list == NULL)
                return NULL;
        return (list->tail);
}

void slist_append(struct slist *list, struct slnode *node)
{
	if (list == NULL || node == NULL)
		return;

        node->next = NULL;

        if (list->tail == NULL)
                list->head = node;
        else
                list->tail->next = node;
        list->tail = node;

        ++(list->size);
}

void slist_prepend(struct slist *list, struct slnode *node)
{
	if (list == NULL || node == NULL)
		return;

        node->next = list->head;
        list->head = node;

        if (list->tail == NULL)
                list->tail = node;

        ++(list->size);
}

void slist_remove(struct slist *list, struct slnode *node)
{
        struct slnode *prev;

	if (list == NULL || node == NULL)
		return;

        prev = list->head;
        while ((prev != NULL) && (prev->next != node))
                prev = prev->next;

        if (prev != NULL)
                prev->next = node->next;
        if (list->head == node)
                list->head = node->next;
        if (list->tail == node)
                list->tail = prev;

        --(list->size);
}

int slist_size(struct slist *list)
{
        return (list ? list->size : -1);
}

static int slist_strcmp(struct slnode *node, void *ud)
{
        if ((node == NULL) || (ud == NULL))
                return -1;
        return (strcmp(node->key, (const char *)ud));
}

struct slnode *slist_search_custom(struct slist *list, int (*compare)(struct slnode *, void *), void *user_data)
{
        struct slnode *node, *next;

        if (list == NULL)
                return NULL;

	for (node = list->head; node; node = next) {
                next = node->next;
                if (compare(node, user_data) == 0)
                        break;
	}

        return node;
}

struct slnode *slist_search(struct slist *list, const char *key)
{
        return (slist_search_custom(list, slist_strcmp, (void *)key));
}
