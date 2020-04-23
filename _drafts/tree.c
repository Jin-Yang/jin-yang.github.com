/*
 * ##### 94 ##### Binary Tree Inorder Traversal
 * Given a binary tree, return the inorder traversal of its nodes' values.
 *
 * Example:
 *
 * Input: [1,null,2,3]
 *    1
 *     \
 *      2
 *     /
 *    3
 *
 * Output: [1,3,2]
 *
 * Follow up: Recursive solution is trivial, could you do it iteratively?
 */

/* =============================================================================== */
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
#if 0
#include <stdlib.h>


struct TreeNode {
	int val;
	struct TreeNode *left;
	struct TreeNode *right;
};

/* Method #1 递归法 */
void RecursiveInorderTraversal(struct TreeNode *node, int *array, int *offset)
{
	if (node == NULL)
		return;

	RecursiveInorderTraversal(node->left, array, offset);

	array[*offset] = node->val;
	*offset += 1;

        RecursiveInorderTraversal(node->right, array, offset);
}

int *InorderTraversal(struct TreeNode *root, int *returnSize)
{
	int *array, offset = 0;

	if (NULL == root) {
		*returnSize = 0;
		return NULL;
	}

	array = calloc(sizeof(*array), 256);
	if (array == NULL) {
		*returnSize = 0;
		return NULL;
	}

	RecursiveInorderTraversal(root, array, &offset);

	*returnSize = offset;
	return array;
}
#endif



#if 0
//方法二：迭代法
//1,遍历节点A，将节点压入栈中，
//2,遍历A的左支，
//3,A出栈，访问A
//4,遍历A的右支
int *inorderTraversal(struct TreeNode *root, int *returnSize) {
    int     iMax        = 100;
    int     iTop        = 0;
    int*    pRet        = NULL;
    int     iRetSize    = 0;

    struct TreeNode*    pTmp    = root;
    struct TreeNode*    pStrTreeBuf[iMax];       //建立节点指针数组，模拟栈保存节点

    pRet = (int*)malloc(sizeof(int) * iMax);
    memset(pRet, 0x00, sizeof(int) * iMax);

    while((pTmp != NULL) || (iTop != 0))
    {
        while(pTmp != NULL)
        {
            //1,遍历节点，将检点压入栈中
            pStrTreeBuf[iTop] = pTmp;
            iTop += 1;

            //2,遍历左支
            pTmp = pTmp->left;
        }

        //3,出栈，访问节点
        iTop -= 1;
        pTmp = pStrTreeBuf[iTop];
        pRet[iRetSize] = pTmp->val;
        iRetSize += 1;

        //4,遍历右支
        pTmp = pTmp->right;
    }

    //5,返回
    *returnSize = iRetSize;
    return pRet;
}
#endif


#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

struct bstree_node {
	uintptr_t key, value;
	struct bstree_node *left, *right, *parent;
};

struct bstree {
	struct bstree_node *root;
	int (*compare)(const uintptr_t, const uintptr_t);
	int size;
};

struct bstree_node *bstree_node_create(uintptr_t key, uintptr_t value)
{
	struct bstree_node *node;

	node = calloc(1, sizeof(*node));
	if (node == NULL)
		return NULL;
	node->key = key;
	node->value = value;
	return node;
}

void bstree_destroy(struct bstree *tree)
{
	if (tree == NULL)
		return;
	free(tree);
}

struct bstree *bstree_create(int (*cmp)(const uintptr_t, const uintptr_t))
{
	struct bstree *tree;

	tree = calloc(1, sizeof(*tree));
	if (tree == NULL)
		return NULL;
	tree->compare = cmp;
	return tree;
}

static struct bstree_node *bstree_do_search(struct bstree *tree, uintptr_t key,
		struct bstree_node **parent, int *isleft)
{
	int rc;
	struct bstree_node *curr;

	*isleft = 0;
	*parent = NULL;
	curr = tree->root;
	while (curr != NULL) {
		*parent = curr;
		rc = tree->compare(key, curr->key);
		if (rc == 0) {
			return curr;
		} else if (rc > 0) {
			*isleft = 0;
			curr = curr->right;
		} else {
			*isleft = 1;
			curr = curr->left;
		}
	}

	return NULL;
}

struct bstree_node *bstree_insert(struct bstree *tree, uintptr_t key, uintptr_t value)
{
	int isleft;
	struct bstree_node *node, *parent;

	if (tree == NULL)
		return NULL;

	node = bstree_do_search(tree, key, &parent, &isleft);
	if (node != NULL)
		return node;

	node = bstree_node_create(key, value);
	if (node == NULL)
		return NULL;

	node->parent = parent;
	if (parent == NULL)
		tree->root = node;
	else if (isleft > 0)
		parent->left = node;
	else
		parent->right = node;

	return node;
}

int bstree_search(struct bstree *tree, uintptr_t key, uintptr_t *ret)
{
	int isleft;
	struct bstree_node *node, *tmp;

	if (tree == NULL)
		return -EINVAL;

	node = bstree_do_search(tree, key, &tmp, &isleft);
	if (node == NULL)
		return -1;

	*ret = node->value;
	return 0;
}

void bstree_preorder(struct bstree_node *node)
{
	if (node == NULL)
		return;
	//printf("Key=%ld Value=%ld\n", node->key, node->value);
	printf("%c ", (char)node->key);
	bstree_preorder(node->left);
	bstree_preorder(node->right);
}

void bstree_inorder(struct bstree_node *node)
{
	if (node == NULL)
		return;
	bstree_inorder(node->left);
	//printf("Key=%ld Value=%ld\n", node->key, node->value);
	printf("%c ", (char)node->key);
	bstree_inorder(node->right);
}

void bstree_postorder(struct bstree_node *node)
{
	if (node == NULL)
		return;
	bstree_postorder(node->left);
	bstree_postorder(node->right);
	//printf("Key=%ld Value=%ld\n", node->key, node->value);
	printf("%c ", (char)node->key);
}


#if 0
node *new(){
	return (node *)malloc(sizeof(node));
}
node *root(int value){            //call at the time of initilisation
	node *ptr = new();
	ptr-> data = value;
	ptr-> father = 0;
	ptr-> lptr = 0;
	ptr-> lptr = 0;
	return ptr;
}
void insert(int x, node *ptr){
	if(x > ptr->data){
		if( ptr-> rptr){
			insert(x,ptr-> rptr);
		}else{
			node *temp = new();
			temp-> data = x;
			temp-> father = ptr;
			temp-> lptr = 0;
			temp-> rptr = 0;
			ptr-> rptr = temp;
		}
	}else{
		if( ptr-> lptr){
			insert(x,ptr-> lptr);
		}else{
			node *temp = new();
			temp-> data = x;
			temp-> father = ptr;
			temp-> lptr = 0;
			temp-> rptr = 0;
			ptr-> lptr = temp;
		}
	}
}
void preorder(node *ptr){
	if(ptr){
		printf("%d\n",ptr-> data );
		preorder( ptr-> lptr);
		preorder( ptr-> rptr);
	}
}
void postorder(node *ptr){
	if(ptr){
		postorder( ptr-> lptr);
		postorder( ptr-> rptr);
		printf("%d\n",ptr-> data );
	}
}
void inorder(node *ptr){
	if(ptr){
		inorder( ptr-> lptr);
		printf("%d\n",ptr-> data );
		inorder( ptr-> rptr);
	}
}
node *search(int x, node *ptr){
	if(ptr){
		int y = ptr-> data;
		if(y == x) return ptr;
		else if( y < x ) return search(x,ptr->rptr);
		else return search(x,ptr->lptr);
	}else return 0;
}
node *parent(int x, node *ptr){
	node *temp;
	if(temp = search(x,ptr)){
		return temp-> father;
	}else return 0;
}
node *lchild(int x, node *ptr){
	node *temp;
	if(temp = search(x,ptr)){
		return temp-> lptr;
	}else return 0;
}
node *rchild(int x, node *ptr){
	node *temp;
	if(temp = search(x,ptr)){
		return temp-> rptr;
	}else return 0;
}
int delete(int x, node *ptr){
	node *temp;
	if(temp = search(x,ptr)){
		if(temp == ptr){						   /// ROOT
			node *z = ptr-> rptr;
			ptr-> rptr-> father = 0;
			while( z-> lptr) z = z-> lptr;
			z-> lptr = ptr-> lptr;
			rut = ptr-> rptr;
			free(ptr);
		}else if(ptr-> lptr && ptr-> rptr){		   /// Both child
			node *z,*temp = ptr-> father;
			temp-> rptr = ptr-> rptr;
			ptr-> rptr-> father = temp;
			z = temp-> rptr;
			while( z-> lptr) z = z-> lptr;
			z-> lptr = ptr-> lptr;
			free(ptr);
		}else if(ptr-> lptr && !ptr-> rptr){       /// Left child only
			node *temp = ptr-> father;
			temp-> lptr = ptr-> lptr;
			ptr-> lptr-> father = temp;
			free(ptr);
		}else if(!ptr-> lptr && ptr-> rptr){	   /// Right child only
			node *temp = ptr-> father;
			temp-> rptr = ptr-> rptr;
			ptr-> rptr-> father = temp;
			free(ptr);
		}else{                                     /// No child
			node *temp = ptr-> father;
			int k = temp->data;
			if(k < x) temp-> rptr = 0;
			else temp-> lptr = 0;
			free(ptr);
		}
		return(x);
	}else{
		printf("NOT FOUND\n");
		return 0;
	}
}
int height(node *ptr,int count){
	if(ptr){
		int x = height(ptr-> lptr,count+1),y = height(ptr-> rptr,count+1);
		return ( x > y ? x : y);
	}
	return count;
}

int main(void)
{
	rut = root(100);
	insert(95,rut);
	insert(45,rut);
	insert(195,rut);
	insert(145,rut);
	printf("height %d\n",height(rut,0));
	inorder(rut);
	postorder(rut);
	preorder(rut);
	printf("%d\n",search(95,rut)->data);
	printf("%d\n",lchild(100,rut)->data);
	printf("%d\n",rchild(100,rut)->data);
	printf("%d\n",delete(100,rut));
	preorder(rut);
	printf("height %d\n",height(rut,0));
	return 0;
}
#endif

static int compare_function(const uintptr_t a0, const uintptr_t a1)
{
	return a0 - a1;
}

int main(void)
{
	struct bstree *tree;
	struct bstree_node *nA = bstree_node_create('A', 'A');
	struct bstree_node *nB = bstree_node_create('B', 'B');
	struct bstree_node *nC = bstree_node_create('C', 'C');
	struct bstree_node *nD = bstree_node_create('D', 'D');
	struct bstree_node *nE = bstree_node_create('E', 'E');
	struct bstree_node *nF = bstree_node_create('F', 'F');
	struct bstree_node *nG = bstree_node_create('G', 'G');
	struct bstree_node *nH = bstree_node_create('H', 'H');
	struct bstree_node *nI = bstree_node_create('I', 'I');

	tree = bstree_create(compare_function);
	if (tree == NULL)
		return -1;
	nA->left = nB; nA->right = nC;
	nB->left = nD; nB->right = nE;
	nE->left = nG; nE->right = nH;
	nC->left = nF; nF->right = nI;

	bstree_preorder(nA);  // A B D E G H C F I
	puts("");
	bstree_inorder(nA);   // D B G E H A F I C
	puts("");
	bstree_postorder(nA); // D G H E B I F C A
	puts("");


#if 0
	n1 = bstree_insert(tree, 1, 100);
	assert(n1 != NULL);
	n2 = bstree_insert(tree, 1, 100);
	assert(n1 == n2);
#endif

	bstree_destroy(tree);

	return 0;
}

