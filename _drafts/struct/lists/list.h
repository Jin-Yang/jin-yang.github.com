
#ifndef LIBUTIL_LIST_H_
#define LIBUTIL_LIST_H_ 1

typedef struct llentry_s {
        char                  *key;
        void                  *value;
        struct llentry_s      *next;
} llentry_t;

typedef struct llist_s {
        llentry_t             *head;
        llentry_t             *tail;
        int                    size;
} llist_t;


llist_t *llist_create(void);
void llist_destroy(llist_t *l);

llentry_t *llentry_create(char *key, void *value);
void llentry_destroy(llentry_t *e);

void llist_append(llist_t *l, llentry_t *e);
void llist_prepend(llist_t *l, llentry_t *e);
void llist_remove(llist_t *l, llentry_t *e);

int llist_size(llist_t *l);

llentry_t *llist_search(llist_t *l, const char *key);
llentry_t *llist_search_custom(llist_t *l, int (*compare)(llentry_t *, void *), void *user_data);

llentry_t *llist_head(llist_t *l);
llentry_t *llist_tail(llist_t *l);









/* single linked list */
struct slnode {
        char           *key;
        void           *value;
        struct slnode  *next;
};

struct slist {
        struct slnode *head, *tail;
        int           size;
};

struct slnode *slnode_create(char *key, void *value);
void slnode_destory(struct slnode *node);
struct slist *slist_create(void);
void slist_destroy(struct slist *list);
struct slnode *slist_head(struct slist *list);
struct slnode *slist_tail(struct slist *list);
void slist_append(struct slist *list, struct slnode *node);
void slist_prepend(struct slist *list, struct slnode *node);
void slist_remove(struct slist *list, struct slnode *node);
int slist_size(struct slist *list);

struct slnode *slist_search_custom(struct slist *list, int (*compare)(struct slnode *, void *), void *user_data);
struct slnode *slist_search(struct slist *list, const char *key);

#endif
