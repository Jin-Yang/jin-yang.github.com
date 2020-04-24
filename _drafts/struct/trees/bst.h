#ifndef LIBUTILS_STRUCT_TREES_BST_H_
#define LIBUTILS_STRUCT_TREES_BST_H_ 1

#include <stdint.h>
#include <stddef.h>


#ifndef offsetof
#define offsetof(type, member) ((size_t)&((type *)0)->member)
#endif

#define bstree_container_of(node, type, member)	             \
	((type *)((char *)(node) - offsetof(type, member)))

struct bstree_node {
	struct bstree_node *left, *right;
	unsigned left_is_thread:1;
	unsigned right_is_thread:1;
};

typedef int (*bstree_cmp_t)(const struct bstree_node *, const struct bstree_node *);

struct bstree {
        struct bstree_node *root, *first, *last;
	bstree_cmp_t compare;
        int size;
};

int bstree_init(struct bstree *tree, bstree_cmp_t cmp);
struct bstree_node *bstree_insert(struct bstree *tree, struct bstree_node *node);
struct bstree_node *bstree_get(const struct bstree *tree, const struct bstree_node *key);

#endif
