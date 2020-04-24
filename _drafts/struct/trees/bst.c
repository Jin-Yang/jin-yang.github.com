#include <stdlib.h>

#include "bst.h"

static inline void set_left(struct bstree_node *l, struct bstree_node *n)
{
	n->left = l;
	n->left_is_thread = 0;
}

static inline void set_right(struct bstree_node *r, struct bstree_node *n)
{
	n->right = r;
	n->right_is_thread = 0;
}

static inline void set_prev(struct bstree_node *t, struct bstree_node *n)
{
	n->left = t;
	n->left_is_thread = 1;
}

static inline void set_next(struct bstree_node *t, struct bstree_node *n)
{
	n->right = t;
	n->right_is_thread = 1;
}

static inline struct bstree_node *get_left(const struct bstree_node *n)
{
	if (n->left_is_thread)
		return NULL;
	return n->left;
}

static inline struct bstree_node *get_right(const struct bstree_node *n)
{
	if (n->right_is_thread)
		return NULL;
	return n->right;
}

static inline struct bstree_node *get_prev(const struct bstree_node *n)
{
	if (!n->left_is_thread)
		return NULL;
	return n->left;
}

static inline struct bstree_node *get_next(const struct bstree_node *n)
{
	if (!n->right_is_thread)
		return NULL;
	return n->right;
}

static inline struct bstree_node *get_first(struct bstree_node *node)
{
	struct bstree_node *left;
	while ((left = get_left(node)))
		node = left;
	return node;
}

static inline struct bstree_node *get_last(struct bstree_node *node)
{
	struct bstree_node *right;
	while ((right = get_right(node)))
		node = right;
	return node;
}

struct bstree_node *bstree_first(const struct bstree *tree)
{
	if (tree->root)
		return tree->first;
	return NULL;
}

struct bstree_node *bstree_last(const struct bstree *tree)
{
	if (tree->root)
		return tree->last;
	return NULL;
}

struct bstree_node *bstree_next(const struct bstree_node *node)
{
	struct bstree_node *right = get_right(node);
	if (right)
		return get_first(right);
	return get_next(node);
}

struct bstree_node *bstree_prev(const struct bstree_node *node)
{
	struct bstree_node *left = get_left(node);
	if (left)
		return get_last(left);
	return get_prev(node);
}

int bstree_init(struct bstree *tree, bstree_cmp_t cmp)
{
	if (tree == NULL)
		return 0;

	tree->size = 0;
	tree->compare = cmp;
	return 0;
}

static struct bstree_node *do_lookup(const struct bstree *tree, 
	const struct bstree_node *key, struct bstree_node **pparent, int *is_left)
{
	int rc;
	struct bstree_node *node;

	*is_left = 0;
	*pparent = NULL;
	node = tree->root;
	while (node) {
		rc = tree->compare(node, key);
		if (rc == 0)
			return node;
		*pparent = node;
		*is_left = rc > 0;
		if (rc > 0)
			node = get_left(node);
		else
			node = get_right(node);
	}

	return NULL;
}

static inline void node_do_init(struct bstree_node *node)
{
	node->left = NULL;
	node->right = NULL;
	node->left_is_thread = 0;
	node->right_is_thread = 0;
}

struct bstree_node *bstree_insert(struct bstree *tree, struct bstree_node *node)
{
	int is_left;
	struct bstree_node *key, *parent;

	key = do_lookup(tree, node, &parent, &is_left);
	if (key != NULL)
		return key;

	if (parent == NULL) { /* tree is empty */
		node_do_init(node);
		tree->root = tree->first = tree->last = node;
		return NULL;
	}

	if (is_left) {
		if (parent == tree->first)
			tree->first = node;
		set_prev(get_prev(parent), node);
		set_next(parent, node);
		set_left(node, parent);
	} else {
		if (parent == tree->last)
			tree->last = node;
		set_prev(parent, node);
		set_next(get_next(parent), node);
		set_right(node, parent);
	}

	return NULL;
}

struct bstree_node *bstree_get(const struct bstree *tree, const struct bstree_node *key)
{
	int is_left;
	struct bstree_node *parent;

	return do_lookup(tree, key, &parent, &is_left);
}

static void set_child(struct bstree_node *child, struct bstree_node *node, int left)
{
	if (left)
		set_left(child, node);
	else
		set_right(child, node);
}

void bstree_remove(struct bstree_node *node, struct bstree *tree)
{
#if 0
	int is_left;
	struct bstree_node *left, *right, *next;
	struct bstree_node *parent, fake_parent;

	do_lookup(tree, node, &parent, &is_left);
	if (parent == NULL) {
		node_do_init(&fake_parent);
		parent = &fake_parent;
		is_left = 0;
	}

	left  = get_left(node);
	right = get_right(node);
	if (left == NULL && right == NULL) {
		if (is_left)
			set_prev(get_prev(node), parent);
		else
			set_next(get_next(node), parent);
		next = parent;

		goto update_first_last;
	}

	if (left == NULL) {
		next = get_first(right);
		set_prev(get_prev(node), next);
		set_child(right, parent, is_left);
		goto update_first_last;
	}

	if (right == NULL) {
		next = get_last(left);
		set_next(get_next(node), next);
		set_child(left, parent, is_left);
		goto update_first_last;
	}

	next = get_first(right);
	if (next != right) {
		/* 'm' is the parent of 'next' */
		struct bstree_node *m = get_next(get_last(next));

		if (get_right(next))
			set_left(get_right(next), m);
		else
			set_prev(next, m);

		set_right(right, next);
	}
	set_child(next, parent, is_left);
	set_left(left, next);
	set_next(next, get_last(left));
out:
	if (parent == &fake_parent)
		tree->root = get_right(parent);
	return;

update_first_last:
	if (node == tree->first)
		tree->first = next;
	if (node == tree->last)
		tree->last = next;
	goto out;
#endif
}

#if 0
void bstree_replace(struct bstree_node *old, struct bstree_node *new,
		    struct bstree *tree)
{
	struct bstree_node *parent, *next, *prev;
	int is_left;

	if (tree->first == old)
		tree->first = new;
	if (tree->last == old)
		tree->last = new;
	if (tree->root == old)
		tree->root = new;
	else {
		/*
		 * Update the parent: do a full lookup to retrieve
		 * it. There's probably a better way but it's bst...
		 */
		do_lookup(old, tree, &parent, &is_left);
		if (parent)
			set_child(new, parent, is_left);
	}

	/* update the thread links */
	prev = bstree_prev(old);
	if (prev && get_next(prev) == old)
		set_next(new, prev);
	next = bstree_next(old);
	if (next && get_prev(next) == old)
		set_prev(new, next);

	*new = *old;
}
#endif

