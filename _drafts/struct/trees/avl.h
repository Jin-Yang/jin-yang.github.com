#ifndef LIBUTILS_STRUCT_TREES_AVL_H_
#define LIBUTILS_STRUCT_TREES_AVL_H_ 1

/* private data types */
struct avlnode {
        void           *key;
        void           *value;

        struct avlnode *left;
        struct avlnode *right;
        struct avlnode *parent;
        int             height;
};

struct avltree {
        struct avlnode  *root;
        int            (*compare)(const void *, const void *);
        int              size;
};

struct avliter {
        struct avltree *tree;
        struct avlnode *node;
};
typedef int (*avltree_compare)(const void *, const void *);

/*
 * Allocates a new AVL-tree.
 *
 * compare  The function-pointer is used to compare two keys. It has to
 *          return less than zero if its first argument is smaller then
 *          the second argument, more than zero if the first argument
 *          is bigger than the second argument and zero if they are equal.
 *          If your keys are char-pointers, you can use the `strcmp'
 *          function from the libc here.
 */
struct avltree *avltree_create(int (*compare)(const void *, const void *));

/*
 * Deallocates an AVL-tree.
 * NOTE: Stored Key-Value pointer are lost, but not freed.
 */
void avltree_destroy(struct avltree *tree);

/*
 * Stores the key-value-pair in the AVL-tree.
 *
 * NOTE: Key used to store the value under. This is used to get back to
 *       the value again. The pointer is stored in an internal structure
 *       and _not_ copied. So the memory pointed to may _not_ be freed
 *       before this entry is removed. You can use the `rkey' argument
 *       to `avltree_remove' to get the original pointer back and free it.
 *
 * RETURN VALUE
 *   Zero upon success, non-zero otherwise. It's less than zero if an error
 *   occurred or greater than zero if the key is already stored in the tree.
 */
int avltree_insert(struct avltree *tree, void *key, void *value);

/*
 * Removes a key-value-pair from the tree t. The stored key and value may be
 * returned in 'rkey' and 'rvalue'.
 *
 * NOTE: Pointer to a pointer in which to store the key. May be NULL.
 *       Since the `key' pointer is not copied when creating an entry,
 *       the pointer may not be available anymore from outside the tree.
 *       You can use this argument to get the actual pointer back and
 *       free the memory pointed to by it.
 *
 * RETURN VALUE
 *   Zero upon success or non-zero if the key isn't found in the tree.
 */
int avltree_remove(struct avltree *tree, const void *key, void **rkey, void **rvalue);

/*
 * Retrieve the 'value' belonging to 'key'.
 *
 * RETURN VALUE
 *   Zero upon success or non-zero if the key isn't found in the tree.
 */
int avltree_get(struct avltree *tree, const void *key, void **value);

/*
 * Remove a (pseudo-)random element from the tree and return its 'key' and
 * 'value'. Entries are not returned in any particular order. This function
 * is intended for cache-flushes that don't care about the order but simply
 * want to remove all elements, one at a time.
 *
 * RETURN VALUE
 *   Zero upon success or non-zero if the tree is empty or key or value is
 *   NULL.
 */
int avltree_pick(struct avltree *tree, void **key, void **value);

int avltree_size(struct avltree *tree);

struct avliter *avltree_get_iterator(struct avltree *tree);
void avltree_iterator_destroy(struct avliter *iter);
int avltree_iterator_next(struct avliter *iter, void **key, void **value);
int avltree_iterator_prev(struct avliter *iter, void **key, void **value);

#endif
