#ifndef GRAPH_VIS_BT
#define GRAPH_VIS_BT

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "node.h"
#include "queue.h"

DEFINE_LIST(Node*, NList)
DEFINE_QUEUE(Node*, NQueue)

typedef enum TreeType {
	BINARY, BINARY_SEARCH, RED_BLACK
} TreeType;

typedef struct BT {
	Node* root;
	size_t size;
} BT;

// bt
Node* BT_Build(int preorder[], int inorder[], int ilower, int iupper, int index);
BT       BT_Init(int preorder[], int inorder[], size_t len);
size_t   BT_MaxDepth(BT* tree);
void     BT_Add(int val, BT* tree);
void     BT_Transplant(Node* root, Node* node, Node* newNode);
void     BT_Del(Node* node, BT* tree);
void     BT_DelByVal(int val, BT* tree);

// bst
Node* BST_Build(int preorder[], int index, int upper);
BT       BST_Init(int preorder[], size_t len);
Node* BST_Search(int val, BT* bt);
void     BST_Add(int val, BT* bt);
void     BST_Del(Node* node, BT* bt);

// rb
Node* RB_NodeAlloc(int val);
BT RB_Init(int preorder[], size_t len);
void RB_ResetSentinel();
void RB_LeftRotate(Node* node, BT* bt);
void RB_AddFixup(Node* node, BT* bt);
void RB_RightRotate(Node* node, BT* bt);
void RB_DelFixup(Node* fixupNode, BT* bt);
void RB_Transplant(Node* oldNode, Node* newNode, BT* bt);
void RB_Add(int val, BT* bt);
void RB_Del(Node* node, BT* bt);

uint16_t BT_Depth(const Node* root);
void     BT_Print(const BT* bt);

#endif
