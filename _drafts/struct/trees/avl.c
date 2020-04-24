#include <assert.h>
#include <stdlib.h>

#include "avl.h"

#define ISBALANCE(n) ((((n)->left == NULL) ? 0 : (n)->left->height) - (((n)->right == NULL) ? 0 : (n)->right->height))

#ifdef __DEBUG__
static void verify_tree (struct avlnode *n)
{
	if (n == NULL)
		return;

	verify_tree(n->left);
	verify_tree(n->right);

	assert((ISBALANCE (n) >= -1) && (ISBALANCE (n) <= 1));
	assert((n->parent == NULL) || (n->parent->right == n) || (n->parent->left == n));
}
#else
#define verify_tree(n)
#endif

static void free_node(struct avlnode *n)
{
        if (n == NULL)
                return;

        if (n->left != NULL)
                free_node(n->left);
        if (n->right != NULL)
                free_node(n->right);

        free(n);
}

struct avltree *avltree_create(int (*compare)(const void *, const void *))
{
        struct avltree *t;

        if (compare == NULL)
                return (NULL);

        t = (struct avltree *)malloc(sizeof(*t));
        if (t == NULL)
                return NULL;

        t->root = NULL;
        t->compare = compare;
        t->size = 0;

        return t;
}

void avltree_destroy(struct avltree *t)
{
        if (t == NULL)
                return;
        free_node(t->root);
        free(t);
}

int avltree_size(struct avltree *t)
{
        if (t == NULL)
                return 0;
        return t->size;
}

static struct avlnode *search(struct avltree *t, const void *key)
{
        struct avlnode *n;
        int cmp;

        n = t->root;
        while (n != NULL) {
                cmp = t->compare(key, n->key);
                if (cmp == 0)
                        return n;
                else if (cmp < 0)
                        n = n->left;
                else
                        n = n->right;
        }

        return NULL;
}

static int calc_height(struct avlnode *n)
{
        int height_left;
        int height_right;

        if (n == NULL)
                return 0;

        height_left = (n->left == NULL) ? 0 : n->left->height;
        height_right = (n->right == NULL) ? 0 : n->right->height;

        return (((height_left > height_right) ? height_left : height_right) + 1);
}

/*
 *         (x)             (y)
 *        /   \           /   \
 *     (y)    /\         /\    (x)
 *    /   \  /_c\  ==>  / a\  /   \
 *   /\   /\           /____\/\   /\
 *  / a\ /_b\               /_b\ /_c\
 * /____\
 *
 */
static struct avlnode *rotate_right(struct avltree *t, struct avlnode *x)
{
        struct avlnode *p;
        struct avlnode *y;
        struct avlnode *b;

        assert(x != NULL);
        assert(x->left != NULL);

        p = x->parent;
        y = x->left;
        b = y->right;

        x->left = b;
        if (b != NULL)
                b->parent = x;

        x->parent = y;
        y->right = x;

        y->parent = p;
        assert((p == NULL) || (p->left == x) || (p->right == x));
        if (p == NULL)
                t->root = y;
        else if (p->left == x)
                p->left = y;
        else
                p->right = y;

        x->height = calc_height(x);
        y->height = calc_height(y);

        return y;
}

/*
 *    (x)                   (y)
 *   /   \                 /   \
 *  /\    (y)           (x)    /\
 * /_a\  /   \   ==>   /   \  / c\
 *      /\   /\       /\   /\/____\
 *     /_b\ / c\     /_a\ /_b\
 *         /____\
 *
 */
static struct avlnode *rotate_left(struct avltree *t, struct avlnode *x)
{
        struct avlnode *p;
        struct avlnode *y;
        struct avlnode *b;

        assert(x != NULL);
        assert(x->right != NULL);

        p = x->parent;
        y = x->right;
        b = y->left;

        x->right = b;
        if (b != NULL)
                b->parent = x;

        x->parent = y;
        y->left = x;

        y->parent = p;
        assert((p == NULL) || (p->left == x) || (p->right == x));
        if (p == NULL)
                t->root = y;
        else if (p->left == x)
                p->left = y;
        else
                p->right = y;

        x->height = calc_height(x);
        y->height = calc_height(y);

        return y;
}

static struct avlnode *rotate_left_right(struct avltree *t, struct avlnode *x)
{
        rotate_left(t, x->left);
        return rotate_right(t, x);
}

static struct avlnode *rotate_right_left(struct avltree *t, struct avlnode *x)
{
        rotate_right(t, x->right);
        return rotate_left(t, x);
}

static void rebalance(struct avltree *t, struct avlnode *n)
{
        int b_top;
        int b_bottom;

        while (n != NULL) {
                b_top = ISBALANCE(n);
                assert((b_top >= -2) && (b_top <= 2));

                if (b_top == -2) {        // right is higher
                        assert(n->right != NULL);
                        b_bottom = ISBALANCE(n->right);
                        assert((b_bottom >= -1) && (b_bottom <= 1));
                        if (b_bottom == 1)
                                n = rotate_right_left(t, n);
                        else
                                n = rotate_left(t, n);
                } else if (b_top == 2) {  // left is higher
                        assert(n->left != NULL);
                        b_bottom = ISBALANCE(n->left);
                        assert((b_bottom >= -1) && (b_bottom <= 1));
                        if (b_bottom == -1)
                                n = rotate_left_right(t, n);
                        else
                                n = rotate_right(t, n);
                } else {
                        int height = calc_height(n);
                        if (height == n->height)
                                break;
                        n->height = height;
                }
                assert(n->height == calc_height(n));

                n = n->parent;
        } /* while (n != NULL) */
}

int avltree_insert(struct avltree *t, void *key, void *value)
{
        struct avlnode *newnode;
        struct avlnode *nptr;
        int cmp;

        newnode = (struct avlnode *)malloc(sizeof(*newnode));
        if (newnode == NULL)
                return -1;

        newnode->key    = key;
        newnode->value  = value;
        newnode->height = 1;
        newnode->left   = NULL;
        newnode->right  = NULL;

        if (t->root == NULL) {
                newnode->parent = NULL;
                t->root = newnode;
                t->size = 1;
                return 0;
        }

        nptr = t->root;
        while (1) {
                cmp = t->compare(nptr->key, newnode->key);
                if (cmp == 0) {
                        free_node(newnode);
                        return 1;
                } else if (cmp < 0) { /* nptr < newnode */
                        if (nptr->right == NULL) {
                                nptr->right = newnode;
                                newnode->parent = nptr;
                                rebalance(t, nptr);
                                break;
                        } else {
                                nptr = nptr->right;
                        }
                } else { /* nptr > newnode */
                        if (nptr->left == NULL) {
                                nptr->left = newnode;
                                newnode->parent = nptr;
                                rebalance(t, nptr);
                                break;
                        } else {
                                nptr = nptr->left;
                        }
                }
        }

        verify_tree(t->root);
        ++t->size;
        return 0;
}

int avltree_get(struct avltree *t, const void *key, void **value)
{
        struct avlnode *n;
        assert(t != NULL);

        n = search(t, key);
        if (n == NULL)
                return -1;

        if (value != NULL)
                *value = n->value;

        return 0;
}

static struct avlnode *avltree_node_next(struct avlnode *n)
{
        struct avlnode *r; /* return node */

        if (n == NULL)
                return NULL;

        /*
         * If we can't descent any further, we have to backtrack to the first
         * parent that's bigger than we, i. e. who's _left_ child we are.
         */
        if (n->right == NULL) {
                r = n->parent;
                while ((r != NULL) && (r->parent != NULL)) {
                        if (r->left == n)
                                break;
                        n = r;
                        r = n->parent;
                }

                /*
                 * n->right == NULL && r == NULL => t is root and has no next
                 * r->left != n => r->right = n => r->parent == NULL
                 */
                if ((r == NULL) || (r->left != n)) {
                        assert((r == NULL) || (r->parent == NULL));
                        return NULL;
                } else {
                        assert(r->left == n);
                        return r;
                }
        } else {
                r = n->right;
                while (r->left != NULL)
                        r = r->left;
        }

        return r;
}

int avltree_pick(struct avltree *t, void **key, void **value)
{
        struct avlnode *n, *p;

        if (t == NULL || t->root == NULL)
                return -1;

        n = t->root;
        while ((n->left != NULL) || (n->right != NULL)) {
                if (n->left == NULL) {
                        n = n->right;
                        continue;
                } else if (n->right == NULL) {
                        n = n->left;
                        continue;
                }

                if (n->left->height > n->right->height)
                        n = n->left;
                else
                        n = n->right;
        }

        p = n->parent;
        if (p == NULL)
                t->root = NULL;
        else if (p->left == n)
                p->left = NULL;
        else
                p->right = NULL;

        if (key)
		*key = n->key;
	if (value)
		*value = n->value;

        free_node(n);
        --t->size;
        rebalance(t, p);

        return 0;
}

static struct avlnode *avltree_node_prev(struct avlnode *n)
{
        struct avlnode *r; /* return node */

        if (n == NULL)
                return NULL;

        /*
         * If we can't descent any further, we have to backtrack to the first
         * parent that's smaller than we, i. e. who's _right_ child we are.
         */
        if (n->left == NULL) {
                r = n->parent;
                while ((r != NULL) && (r->parent != NULL)) {
                        if (r->right == n)
                                break;
                        n = r;
                        r = n->parent;
                }

                /*
                 * n->left == NULL && r == NULL => t is root and has no next
                 * r->right != n => r->left = n => r->parent == NULL
                 */
                if ((r == NULL) || (r->right != n)) {
                        assert((r == NULL) || (r->parent == NULL));
                        return NULL;
                } else {
                        assert(r->right == n);
                        return r;
                }
        } else {
                r = n->left;
                while (r->right != NULL)
                        r = r->right;
        }

        return r;
}

static int _remove(struct avltree *t, struct avlnode *n)
{
        assert((t != NULL) && (n != NULL));

        if ((n->left != NULL) && (n->right != NULL)) {
                struct avlnode *r;    /* replacement node */
                if (ISBALANCE(n) > 0) { /* left subtree is higher */
                        assert(n->left != NULL);
                        r = avltree_node_prev(n);

                } else { /* right subtree is higher */
                        assert(n->right != NULL);
                        r = avltree_node_next(n);
                }

                assert((r->left == NULL) || (r->right == NULL));

                /* copy content */
                n->key = r->key;
                n->value = r->value;

                n = r;
        }

        assert((n->left == NULL) || (n->right == NULL));

        if ((n->left == NULL) && (n->right == NULL)) {
                /* Deleting a leave is easy */
                if (n->parent == NULL) {
                        assert(t->root == n);
                        t->root = NULL;
                } else {
                        assert((n->parent->left == n) || (n->parent->right == n));
                        if (n->parent->left == n)
                                n->parent->left = NULL;
                        else
                                n->parent->right = NULL;

                        rebalance(t, n->parent);
                }

                free_node(n);
        } else if (n->left == NULL) {
                assert(ISBALANCE(n) == -1);
                assert((n->parent == NULL) || (n->parent->left == n) || (n->parent->right == n));
                if (n->parent == NULL) {
                        assert(t->root == n);
                        t->root = n->right;
                } else if (n->parent->left == n) {
                        n->parent->left = n->right;
                } else {
                        n->parent->right = n->right;
                }
                n->right->parent = n->parent;

                if (n->parent != NULL)
                        rebalance(t, n->parent);

                n->right = NULL;
                free_node(n);
        } else if (n->right == NULL) {
                assert(ISBALANCE(n) == 1);
                assert((n->parent == NULL) || (n->parent->left == n) || (n->parent->right == n));
                if (n->parent == NULL) {
                        assert(t->root == n);
                        t->root = n->left;
                } else if (n->parent->left == n) {
                        n->parent->left = n->left;
                } else {
                        n->parent->right = n->left;
                }
                n->left->parent = n->parent;

                if (n->parent != NULL)
                        rebalance(t, n->parent);

                n->left = NULL;
                free_node(n);
        } else {
                assert(0);
        }

        return 0;
}

int avltree_remove(struct avltree *t, const void *key, void **rkey, void **rvalue)
{
        struct avlnode *n;
        int status;

        assert(t != NULL);

        n = search(t, key);
        if (n == NULL)
                return -1;

        if (rkey != NULL)
                *rkey = n->key;
        if (rvalue != NULL)
                *rvalue = n->value;

        status = _remove(t, n);
        verify_tree(t->root);
        --t->size;
        return status;
}

struct avliter *avltree_get_iterator(struct avltree *t)
{
	struct avliter *iter;

	if (t == NULL)
		return NULL;

	iter = calloc(1, sizeof(*iter));
	if (iter == NULL)
		return NULL;
	iter->tree = t;

	return iter;
}

int avltree_iterator_next(struct avliter *iter, void **key, void **value)
{
	struct avlnode *n;

	if (iter == NULL)
		return -1;

	if (iter->node == NULL) {
		for (n = iter->tree->root; n != NULL; n = n->left)
			if (n->left == NULL)
				break;
		iter->node = n;
	} else {
		n = avltree_node_next(iter->node);
	}

	if (n == NULL)
		return -1;

        iter->node = n;
	if (key)
		*key = n->key;
	if (value)
		*value = n->value;

	return 0;
}

int avltree_iterator_prev(struct avliter *iter, void **key, void **value)
{
	struct avlnode *n;

	if ((iter == NULL) || (key == NULL) || (value == NULL))
		return -1;

	if (iter->node == NULL) {
		for (n = iter->tree->root; n != NULL; n = n->left)
			if (n->right == NULL)
				break;
		iter->node = n;
	} else {
		n = avltree_node_prev(iter->node);
	}

	if (n == NULL)
		return -1;

	iter->node = n;
	*key = n->key;
	*value = n->value;

	return 0;
}

void avltree_iterator_destroy(struct avliter *iter)
{
	if (iter)
		free(iter);
}
