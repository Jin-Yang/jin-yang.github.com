#ifndef LIBUTILS_STRUCT_LIST_DOUBLE_H_
#define LIBUTILS_STRUCT_LIST_DOUBLE_H_

struct dlist_node {
        struct dlist_node *next, *prev;
        void *data;
};

struct dlist_header {
        struct dlist_node *head, *tail;
        int size;
};

struct dlist_header *dlist_header_create(void);
void dlist_header_destroy(struct dlist_header *header);
struct dlist_node *dlist_add_tail(struct dlist_header *list, void *data);
struct dlist_node *dlist_add_head(struct dlist_header *list, void *data);
void dlist_destroy(struct dlist_header *list, void (*func)(void *));
void dlist_del_node(struct dlist_header *list, struct dlist_node *node, void (*func)(void *));

#endif
