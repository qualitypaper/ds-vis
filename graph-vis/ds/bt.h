#ifndef GRAPH_VIS_DEFS
#define GRAPH_VIS_DEFS

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct Node {
	struct Node* left;
	struct Node* right;
	struct Node* par;
	int val;
};

DEFINE_LIST(struct Node*, NList)
DEFINE_QUEUE(struct Node*, NQueue)

struct BT {
	struct Node* root;
	size_t size;
};

struct Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index);
struct BT    BT_Init(int preorder[], int inorder[], size_t len);
void         BT_Add(int val, struct BT* tree);
void         BT_Transplant(struct Node* root, struct Node* node, struct Node* newNode);
void         BT_Del(struct Node* node, struct BT* tree);
void         BT_DelByVal(int val, struct BT* tree);

struct Node* BST_Build(int preorder[], size_t index, size_t upper);
struct BT    BST_Init(int preorder[], size_t len);
struct Node* BST_Search(int val, struct BT* bt);
void         BST_Add(int val, struct BT* bt);
void         BST_Del(struct Node* node, struct BT* bt);

uint16_t     BT_Depth(const struct Node* root);
void         BT_Print(const struct Node* root);

#endif
