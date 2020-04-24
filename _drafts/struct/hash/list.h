#ifndef LIBUTILS_STRUCT_HASH_LIST_H_
#define LIBUTILS_STRUCT_HASH_LIST_H_

enum {
        HASH_OPT_CAPACITY,
        HASH_OPT_HASH_FUNC,
        HASH_OPT_ENTRY_DESTORY
};

#define HASH_EXISTS       -2
#define HASH_NOT_EXISTS   -3

struct hash_entry {
        struct hash_entry *next;
        char *key;
        void *value;
};

struct hash_header {
        struct hash_entry *head;
        unsigned int count;
};

struct hash_table {
        struct hash_header *buckets;
        unsigned int size, capacity;

        void (*entry_destroy)(char *, void *);
        unsigned int (*hash_func)(const char *);
};

struct hash_table *hash_create(unsigned int size);
int hash_options(struct hash_table *h, int action, void *data);
int hash_put(struct hash_table *h, char *key, void *value);

void hash_destroy(struct hash_table *h);
void *hash_get(struct hash_table *h, const char *key);
void *hash_del(struct hash_table *h, const char *key);

#endif
