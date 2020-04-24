#include "list.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef HASH_DEFAULT_CAPACITY
#define HASH_DEFAULT_CAPACITY  10
#endif

/* http://www.cse.yorku.ca/~oz/hash.html */
static inline unsigned int hash_default_func(const char *str)
{
        int c;
        unsigned int hash = 5381;

        assert(str);
        while ((c = *str++))
                hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
}

static inline struct hash_entry *create_new_entry(char *key, void *value)
{
        struct hash_entry *ent;

        ent = (struct hash_entry *)malloc(sizeof(*ent));
        if (ent == NULL)
                return NULL;
        ent->next = NULL;
        ent->key = key;
        ent->value = value;

        return ent;
}

struct hash_table *hash_create(unsigned int size)
{
        struct hash_table *hash;

        hash = (struct hash_table *)malloc(sizeof(*hash));
        if (hash == NULL)
                return NULL;

        hash->buckets = (struct hash_header *)calloc(size,
                        sizeof(struct hash_header));
        if (hash->buckets == NULL) {
                free(hash);
                return NULL;
        }
        hash->size = size;
        hash->entry_destroy = NULL;
        hash->hash_func = hash_default_func;
        hash->capacity = HASH_DEFAULT_CAPACITY;

        return hash;
}
int hash_options(struct hash_table *h, int action, void *data)
{
        if (h == NULL) {
                errno = EINVAL;
                return -1;
        }

        switch (action) {
        case HASH_OPT_CAPACITY:
                h->capacity = *(unsigned int *)data;
                return 0;
        case HASH_OPT_HASH_FUNC:
                h->hash_func = (unsigned int(*)(const char *))data;
                return 0;
        case HASH_OPT_ENTRY_DESTORY:
                h->entry_destroy = (void(*)(char *, void *))data;
                return 0;
        }

        errno = EINVAL;
        return -1;
}

void hash_destroy(struct hash_table *h)
{
        unsigned int i;
        struct hash_entry *curr, *next;

        if (h == NULL)
                return;
        if (h->buckets == NULL) {
                free(h);
                return;
        }

        for (i = 0; i < h->size; i++) {
                for (curr = h->buckets[i].head; curr; curr = next) {
                        next = curr->next;
                        if (h->entry_destroy)
                                (*h->entry_destroy)(curr->key, curr->value);
                        free(curr);
                }
        }

        free(h->buckets);
        free(h);
}
static int hash_expend(struct hash_table *h)
{
        unsigned int i, idx, newsz;
        struct hash_header *headers;
        struct hash_entry *curr, *next;

        newsz = h->size * 2;
        headers = (struct hash_header *)calloc(newsz, sizeof(*headers));
        if (headers == NULL)
                return -1;

        for (i = 0; i < h->size; i++) {
                for (curr = h->buckets[i].head; curr; curr = next) {
                        next = curr->next;

                        idx = (*h->hash_func)(curr->key) % newsz;
                        if (headers[idx].head == NULL)
                                curr->next = NULL;
                        else
                                curr->next = headers[idx].head;
                        headers[idx].head = curr;
                        headers[idx].count++;
                }
        }

        free(h->buckets);
        h->buckets = headers;

        return 0;
}


int hash_put(struct hash_table *h, char *key, void *value)
{
        struct hash_header *header;
        struct hash_entry *curr, *ent;

        header = h->buckets + (h->hash_func(key) % h->size);
        if (header->head == NULL) {
                ent = create_new_entry(key, value);
                if (ent == NULL)
                        return -1;
                header->head = ent;
                header->count = 1;
                return 0;
        }

        /* replace it if entry already exists. */
        for (curr = header->head; curr; curr = curr->next) {
                if (strcmp(curr->key, key))
                        continue;

                if (h->entry_destroy)
                        (*h->entry_destroy)(curr->key, curr->value);
                curr->key = key;
                curr->value = value;
                return 0;
        }

        /* add to head. */
        ent = create_new_entry(key, value);
        if (ent == NULL)
                return -1;
        ent->next = header->head;
        header->head = ent;
        header->count++;

        if (header->count > h->capacity)
                return hash_expend(h);

        return 0;
}

void *hash_get(struct hash_table *h, const char *key)
{
        struct hash_header *header;
        struct hash_entry *entry;

        header = h->buckets + (h->hash_func(key) % h->size);
        if (header == NULL)
                return NULL;

        for (entry = header->head; entry; entry = entry->next)
                if (strcmp(entry->key, key) == 0)
                        return entry->value;

        return NULL;
}

void *hash_del(struct hash_table *h, const char *key)
{
	void *data;
        struct hash_header *header;
        struct hash_entry *curr, *prev = NULL;

        header = h->buckets + (h->hash_func(key) % h->size);
        if (header == NULL)
                return NULL;

        for (curr = header->head; curr; curr = curr->next) {
                if (strcmp(curr->key, key) == 0) {
                        if (prev == NULL)
                                header->head = curr->next;
                        else
                                prev->next = curr->next;
			data = curr->value;
                        free(curr);
                        header->count--;
                        return data;
                }
                prev = curr;
        }

        return NULL;
}
